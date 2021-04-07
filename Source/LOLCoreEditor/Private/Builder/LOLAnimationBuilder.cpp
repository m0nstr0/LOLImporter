#include "Builder/LOLAnimationBuilder.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimTypes.h"
#include "Util/LOLConverter.h"

namespace LOLImporter 
{
	bool FLOLAnimationBuilder::BuildAssets(const FLOLAnimationAsset& Asset, TArray<UObject*>& OutAssets)
	{
		UAnimSequence* AnimSequence = NewObject<UAnimSequence>(Asset.Parent, Asset.Name, Asset.Flags);

		if (AnimSequence == nullptr)
		{
			return false;
		}

		const FLOLAnimation& Animation = Asset.Animation;

		AnimSequence->CleanAnimSequenceForImport();
		AnimSequence->RemoveAllTracks();

		USkeleton* Skeleton = Asset.Skeleton.Get(false);

		if (Skeleton == nullptr) 
		{
			return false;
		}

		AnimSequence->SetSkeleton(Skeleton);
		AnimSequence->SetRawNumberOfFrame(Animation.Frames.Num());
		AnimSequence->SequenceLength = Animation.Duration;
		AnimSequence->AdditiveAnimType = EAdditiveAnimationType::AAT_None;

		const FReferenceSkeleton& ReferenceSkeleton = Skeleton->GetReferenceSkeleton();

		TMap<uint32, FName> JointHashToName;
		TMap<uint32, FTransform> JointHashToTransform;
		for (int32 Idx = 0; Idx < ReferenceSkeleton.GetNum(); Idx++)
		{
			FName JointName = ReferenceSkeleton.GetBoneName(Idx);
			FString JointNameStr = JointName.ToString().ToLower();

			uint32 Hash = 0;
			uint32 High = 0;
			for (int32 i = 0; i < JointNameStr.Len(); i++) {
				Hash = (Hash << 4) + ((uint8)JointNameStr[i]);

				if ((High = Hash & 0xF0000000) != 0) {
					Hash ^= (High >> 24);
				}

				Hash &= ~High;
			}

			JointHashToName.Add(Hash, JointName);
			JointHashToTransform.Add(Hash, ReferenceSkeleton.GetRefBonePose()[Idx].Inverse());
		}

		TMap<uint32, FInterpCurve<FVector>> Translations;
		TMap<uint32, FInterpCurve<FVector>> Scales;
		TMap<uint32, FInterpCurve<FQuat>> Rotations;

		for (uint32 JointHash : Animation.JointHashes) {
			FInterpCurve<FVector> TranslationCurve;
			FInterpCurve<FVector> ScaleCurve;
			FInterpCurve<FQuat> RotationCurve;

			TranslationCurve.AutoSetTangents();
			ScaleCurve.AutoSetTangents();
			RotationCurve.AutoSetTangents();

			uint32 FrameID = 0;
			for (const TPair<uint32, FLOLAmimationFrame>& Frame : Animation.Frames)
			{
				if (Frame.Value.Contains(JointHash)) {
					if (Frame.Value[JointHash].HasTranslation) {
						TranslationCurve.AddPoint(FrameID, FLOLConverter::ConvertVector(Frame.Value[JointHash].Translation));
					}
					if (Frame.Value[JointHash].HasScale) {
						ScaleCurve.AddPoint(FrameID, FLOLConverter::ConvertVector(Frame.Value[JointHash].Scale));
					}
					if (Frame.Value[JointHash].HasRotation) {
						RotationCurve.AddPoint(FrameID, FLOLConverter::ConvertQuat(Frame.Value[JointHash].Rotation));
					}
				}
				FrameID++;
			}

			Translations.Add(JointHash, TranslationCurve);
			Scales.Add(JointHash, ScaleCurve);
			Rotations.Add(JointHash, RotationCurve);
		}


		for (uint32 BoneHash : Animation.JointHashes) {
			if (!JointHashToName.Contains(BoneHash)) {
				continue;
			}
			FRawAnimSequenceTrack RawAnimSequenceTrack;
			for (int32 FrameIdx = 0; FrameIdx < Animation.Frames.Num(); FrameIdx++) {
				RawAnimSequenceTrack.PosKeys.Add(Translations[BoneHash].Eval(FrameIdx));
				RawAnimSequenceTrack.ScaleKeys.Add(Scales[BoneHash].Eval(FrameIdx));
				RawAnimSequenceTrack.RotKeys.Add(Rotations[BoneHash].Eval(FrameIdx));
			}
			AnimSequence->AddNewRawTrack(JointHashToName[BoneHash], &RawAnimSequenceTrack);
		}

		AnimSequence->MarkRawDataAsModified();

		OutAssets.Add(AnimSequence);
		return true;
	}
}

