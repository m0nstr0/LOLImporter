// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/LOLANMReader.h"

namespace LOLImporter
{
	FVector FLOLANMReader::DecompressVector(FVector Min, FVector Max, uint8 CompressedData[6])
	{
		FVector Uncompressed = Max - Min;
		Uncompressed.X = Uncompressed.X * ((uint16)(CompressedData[0] | CompressedData[1] << 8)) / 65535.0f + Min.X;
		Uncompressed.Y = Uncompressed.Y * ((uint16)(CompressedData[2] | CompressedData[3] << 8)) / 65535.0f + Min.Y;
		Uncompressed.Z = Uncompressed.Z * ((uint16)(CompressedData[4] | CompressedData[5] << 8)) / 65535.0f + Min.Z;

		return Uncompressed;
	}

	FQuat FLOLANMReader::DecompressQuantizedQuaternion(uint8 CompressedTransform[6])
	{
		uint64 Bits = ((uint64)(CompressedTransform[0] | (CompressedTransform[1] << 8))) | ((uint64)(CompressedTransform[2] | (CompressedTransform[3] << 8))) << 16 | ((uint64)(CompressedTransform[4] | (CompressedTransform[5] << 8))) << 32;	
		uint16 MaxIndex = (uint16)((Bits >> 45) & 0x0003u);

		float A = (float)((((uint16)((Bits >> 30) & 0x7FFFu)) / 32767.0) * 1.41421356237 - 1 / 1.41421356237);
		float B = (float)((((uint16)((Bits >> 15) & 0x7FFFu)) / 32767.0) * 1.41421356237 - 1 / 1.41421356237);
		float C = (float)((((uint16)(Bits & 0x7FFFu)) / 32767.0) * 1.41421356237 - 1 / 1.41421356237);
		float D = FMath::Sqrt(FMath::Max<float>(0, 1 - (A * A + B * B + C * C)));

		switch (MaxIndex) {
		case 0: 
			return FQuat(D, A, B, C);
		case 1: 
			return FQuat(A, D, B, C);
		case 2: 
			return FQuat(A, B, D, C);
		default: 
			return FQuat(A, B, C, D);
		}
	}


	bool FLOLANMReader::ReadCompressed(FLOLAnimation& Animation)
	{
		uint32 ResourceSize = 0;
		GetReader().Serialize(&ResourceSize, sizeof(ResourceSize));

		uint32 FormatToken = 0;
		GetReader().Serialize(&FormatToken, sizeof(FormatToken));

		uint32 Flags = 0;
		GetReader().Serialize(&Flags, sizeof(Flags));

		int32 JointCount = 0;
		GetReader().Serialize(&JointCount, sizeof(JointCount));

		int32 FrameCount = 0;
		GetReader().Serialize(&FrameCount, sizeof(FrameCount));

		int32 JumpCacheCount = 0;
		GetReader().Serialize(&JumpCacheCount, sizeof(JumpCacheCount));

		float Duration = 0;
		GetReader().Serialize(&Duration, sizeof(Duration));

		float FPS = 0;
		GetReader().Serialize(&FPS, sizeof(FPS));

		float TransformQuantizationProperties[6];
		GetReader().Serialize(TransformQuantizationProperties, sizeof(TransformQuantizationProperties));

		FVector TranslationMin;
		GetReader().Serialize(&TranslationMin, sizeof(TranslationMin));

		FVector TranslationMax;
		GetReader().Serialize(&TranslationMax, sizeof(TranslationMax));

		FVector ScaleMin;
		GetReader().Serialize(&ScaleMin, sizeof(ScaleMin));

		FVector ScaleMax;
		GetReader().Serialize(&ScaleMax, sizeof(ScaleMax));

		int32 FramesOffset = 0;
		GetReader().Serialize(&FramesOffset, sizeof(FramesOffset));

		int32 JumpCachesOffset = 0;
		GetReader().Serialize(&JumpCachesOffset, sizeof(JumpCachesOffset));

		int32 JointNameHashesOffset = 0;
		GetReader().Serialize(&JointNameHashesOffset, sizeof(JointNameHashesOffset));

		if (GetReader().IsError() || FramesOffset <= 0 || JointNameHashesOffset <= 0 || JointCount <= 0)
		{
			return false;
		}

		GetReader().Seek(JointNameHashesOffset + 12);

		Animation.Duration = Duration;
		Animation.FPS = FPS;
		Animation.JointHashes.Empty(JointCount);

		for (int32 i = 0; i < JointCount; i++)
		{
			uint32 JointHash;
			GetReader().Serialize(&JointHash, sizeof(JointHash));
			Animation.JointHashes.Add(JointHash);

			if (GetReader().IsError())
			{
				return false;
			}
		}

		GetReader().Seek(FramesOffset + 12);

		const uint8 RotationTransformType = 0;
		const uint8 TranslationTransformType = 1;
		const uint8 ScaleTransformType = 2;

		int32 MaxFrames = 0;
		for (int32 i = 0; i < FrameCount; i++)
		{
			uint16 CompressedTime;
			GetReader().Serialize(&CompressedTime, sizeof(CompressedTime));

			uint16 Bits;
			GetReader().Serialize(&Bits, sizeof(Bits));

			uint8 JointHashIndex = (uint8)(Bits & 0x3FFF);
			uint8 TransformType = (uint8)(Bits >> 14);

			uint8 CompressedTransform[6];
			GetReader().Serialize(CompressedTransform, sizeof(CompressedTransform));

			FLOLAnimationTransform& JointTransform = Animation.GetFrameTransform(CompressedTime, JointHashIndex);
			switch (TransformType)
			{
			case RotationTransformType:
				JointTransform.SetRotation(DecompressQuantizedQuaternion(CompressedTransform));
				break;
			case TranslationTransformType:
				JointTransform.SetTranslation(DecompressVector(TranslationMin, TranslationMax, CompressedTransform));
				break;
			case ScaleTransformType:
				JointTransform.SetScale(DecompressVector(ScaleMin, ScaleMax, CompressedTransform));
				break;
			default:
				return false;
			}

			if (GetReader().IsError())
			{
				return false;
			}
		}

		return true;
	}

	bool FLOLANMReader::Read(FLOLAnimation& Animation)
	{
		uint64 Magic = 0;
		GetReader().Serialize(&Magic, sizeof(Magic));

		uint32 Version = 0;
		GetReader().Serialize(&Version, sizeof(Version));


		if (Magic == 0x6D6E616332643372) //compressed
		{
			if (Version != 1)
			{
				return false;
			}

			return ReadCompressed(Animation);
		}

		if (Magic == 0x646d6e6132643372) //uncompressed
		{
			/*if (version == 5)
			{
				ReadV5(br);
			}
			else if (version == 4)
			{
				ReadV4(br);
			}
			else
			{
				ReadLegacy(br);
			}*/
		}

		return false;
	}
}