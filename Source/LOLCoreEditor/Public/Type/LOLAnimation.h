#pragma once

#include "CoreMinimal.h"

namespace LOLImporter
{
	struct FLOLAnimationTransform
	{
		bool HasTranslation;
		bool HasScale;
		bool HasRotation;
		FVector Translation;
		FVector Scale;
		FQuat Rotation;

		FLOLAnimationTransform() : HasRotation(false), HasScale(false), HasTranslation(false) {}

		void SetRotation(FQuat InRotation)
		{
			HasRotation = true;
			Rotation = InRotation;
			Rotation.Normalize();
		}

		void SetTranslation(FVector InTranslation)
		{
			HasTranslation = true;
			Translation = InTranslation;
		}

		void SetScale(FVector InScale)
		{
			HasScale = true;
			Scale = InScale;
		}
	};

	typedef TMap<uint32, FLOLAnimationTransform> FLOLAmimationFrame;

	struct FLOLAnimation
	{
		float Duration;
		float FPS;
		TArray<uint32> JointHashes;
		TSortedMap<uint32, FLOLAmimationFrame> Frames;

		FLOLAnimationTransform& GetFrameTransform(uint32 FrameID, uint32 JointHashIndex)
		{
			uint32 JointHash = JointHashes[JointHashIndex];
			if (Frames.Contains(FrameID))
			{
				if (Frames[FrameID].Contains(JointHash))
				{
					return Frames[FrameID][JointHash];
				}

				return Frames[FrameID].Add(JointHash, FLOLAnimationTransform());
			}

			return Frames.Add(FrameID).Add(JointHash, FLOLAnimationTransform());
		}
	};
}