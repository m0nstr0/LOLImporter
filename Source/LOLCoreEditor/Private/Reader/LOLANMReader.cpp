// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/LOLANMReader.h"
#include "Util/LOLConverter.h"

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

	bool FLOLANMReader::ReadV3(FLOLAnimation& Animation)
	{
		GetReader().Seek(GetReader().Tell() + 4);

		int32 JointCount;
		GetReader().Serialize(&JointCount, sizeof(JointCount));

		int32 FramesCount;
		GetReader().Serialize(&FramesCount, sizeof(FramesCount));

		int32 FPS = 0;
		GetReader().Serialize(&FPS, sizeof(FPS));

		if (GetReader().IsError() || FramesCount <= 0 || JointCount <= 0 || FPS <= 0)
		{
			return false;
		}

		Animation.Duration = 1.0f / FPS * FramesCount;
		Animation.JointHashes.Empty(JointCount);


		for (int32 JointID = 0; JointID < JointCount; JointID++)
		{
			uint8 Name[32];
			GetReader().Serialize(Name, sizeof(Name));
			GetReader().Seek(GetReader().Tell() + 4);
			if (GetReader().IsError()) {
				return false;
			}

			FString JointName = "";
			for (uint8 j = 0; j < 32; j++) {
				if (Name[j] == '\0') {
					break;
				}
				JointName.AppendChar(TCHAR(Name[j]));
			}
			Animation.JointHashes.Add(FLOLConverter::ElfHash(JointName));

			for (int32 FrameID = 0; FrameID < FramesCount; FrameID++)
			{
				FLOLAnimationTransform& AnimationTransform = Animation.GetFrameTransform(FrameID, JointID);
				FVector Translation;
				FQuat Rotation;
				
				GetReader().Serialize(&Rotation.X, sizeof(Rotation.X));
				GetReader().Serialize(&Rotation.Y, sizeof(Rotation.Y));
				GetReader().Serialize(&Rotation.Z, sizeof(Rotation.Z));
				GetReader().Serialize(&Rotation.W, sizeof(Rotation.W));

				GetReader().Serialize(&Translation.X, sizeof(Translation.X));
				GetReader().Serialize(&Translation.Y, sizeof(Translation.Y));
				GetReader().Serialize(&Translation.Z, sizeof(Translation.Z));

				if (GetReader().IsError()) {
					return false;
				}

				AnimationTransform.SetScale(FVector(1.f));
				AnimationTransform.SetRotation(Rotation);
				AnimationTransform.SetTranslation(Translation);
			}
		}

		return true;
	}

	bool FLOLANMReader::ReadV5(FLOLAnimation& Animation)
	{
		GetReader().Seek(GetReader().Tell() + 16);

		int32 JointCount;
		GetReader().Serialize(&JointCount, sizeof(JointCount));

		int32 FramesCount;
		GetReader().Serialize(&FramesCount, sizeof(FramesCount));

		float FrameDuration;
		GetReader().Serialize(&FrameDuration, sizeof(FrameDuration));

		int32 JointHashesOffset;
		GetReader().Serialize(&JointHashesOffset, sizeof(JointHashesOffset));

		GetReader().Seek(GetReader().Tell() + 8);

		int32 TranslationsOffset;
		GetReader().Serialize(&TranslationsOffset, sizeof(TranslationsOffset));

		int32 RotationsOffset;
		GetReader().Serialize(&RotationsOffset, sizeof(RotationsOffset));

		int32 FramesOffset;
		GetReader().Serialize(&FramesOffset, sizeof(FramesOffset));

		if (GetReader().IsError() || FramesOffset <= 0 || RotationsOffset <= 0 || TranslationsOffset <= 0 || JointHashesOffset <= 0 || JointCount <= 0)
		{
			return false;
		}

		Animation.Duration = FramesCount * FrameDuration;

		//Joints
		GetReader().Seek(JointHashesOffset + 12);
		const int32 JOINT_HASHES_COUNT = (FramesOffset - JointHashesOffset) / 4;
		Animation.JointHashes.AddDefaulted(JOINT_HASHES_COUNT);
		for (int32 Idx = 0; Idx < JOINT_HASHES_COUNT; Idx++)
		{
			GetReader().Serialize(&Animation.JointHashes[Idx], sizeof(Animation.JointHashes[Idx]));

			if (GetReader().IsError())
			{
				return false;
			}
		}

		//Translations
		GetReader().Seek(TranslationsOffset + 12);
		const int32 TRANSLATIONS_COUNT = (RotationsOffset - TranslationsOffset) / 12;
		TArray<FVector> Translations;
		Translations.AddDefaulted(TRANSLATIONS_COUNT);
		for (int32 Idx = 0; Idx < TRANSLATIONS_COUNT; Idx++)
		{
			GetReader().Serialize(&Translations[Idx].X, sizeof(Translations[Idx].X));
			GetReader().Serialize(&Translations[Idx].Y, sizeof(Translations[Idx].Y));
			GetReader().Serialize(&Translations[Idx].Z, sizeof(Translations[Idx].Z));

			if (GetReader().IsError())
			{
				return false;
			}		
		}
		
		//Rotations
		const int32 ROTATIONS_COUNT = (JointHashesOffset - RotationsOffset) / 6;
		GetReader().Seek(RotationsOffset + 12);
		TArray<FQuat> Rotations;
		Rotations.AddDefaulted(ROTATIONS_COUNT);
		for (int32 Idx = 0; Idx < ROTATIONS_COUNT; Idx++)
		{
			uint8 CompressedRotation[6];
			GetReader().Serialize(CompressedRotation, sizeof(CompressedRotation));

			if (GetReader().IsError())
			{
				return false;
			}

			Rotations[Idx] = DecompressQuantizedQuaternion(CompressedRotation);		
		}

		//Frames
		GetReader().Seek(FramesOffset + 12);
		for (int32 FrameIdx = 0; FrameIdx < FramesCount; FrameIdx++)
		{
			for (int32 JoinIdx = 0; JoinIdx < JointCount; JoinIdx++)
			{
				uint16 TranslationIdx;
				uint16 ScaleIdx;
				uint16 RotationIdx;
				GetReader().Serialize(&TranslationIdx, sizeof(TranslationIdx));
				GetReader().Serialize(&ScaleIdx, sizeof(ScaleIdx));
				GetReader().Serialize(&RotationIdx, sizeof(RotationIdx));

				if (GetReader().IsError())
				{
					return false;
				}

				FLOLAnimationTransform& JointTransform = Animation.GetFrameTransform(FrameIdx, JoinIdx);
				JointTransform.SetTranslation(Translations[TranslationIdx]);
				JointTransform.SetScale(Translations[ScaleIdx]);
				JointTransform.SetRotation(Rotations[RotationIdx]);
			}
		}

		return true;
	}

	bool FLOLANMReader::ReadCompressed(FLOLAnimation& Animation)
	{	
		GetReader().Seek(GetReader().Tell() + 12);

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

		GetReader().Seek(GetReader().Tell() + 24);

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

		const uint8 ROTATION_TRANSFORM_TYPE = 0;
		const uint8 TRANSLATION_TRANSFORM_TYPE = 1;
		const uint8 SCALE_TRANSFORM_TYPE = 2;

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
			case ROTATION_TRANSFORM_TYPE:
				JointTransform.SetRotation(DecompressQuantizedQuaternion(CompressedTransform));
				break;
			case TRANSLATION_TRANSFORM_TYPE:
				JointTransform.SetTranslation(DecompressVector(TranslationMin, TranslationMax, CompressedTransform));
				break;
			case SCALE_TRANSFORM_TYPE:
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
			if (Version == 5) {
				return ReadV5(Animation);
			}

			if (Version == 3) {
				return ReadV3(Animation);
			}
		}

		return false;
	}
}