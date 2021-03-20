#pragma once

#include "CoreMinimal.h"

namespace LOLImporter
{
    struct FLOLJoint
    {
        int16 ID;
        int16 ParentID;
        uint32 NameHash;
        FVector LocalTranslation;
        FVector LocalScale;
        FQuat LocalRotation;
        FString Name;
    };

    struct FLOLSkeleton
    {
        bool IsMultiRoot;
        TArray<FLOLJoint> Joints;
        TArray<uint16> JointInfluences;
    };
}