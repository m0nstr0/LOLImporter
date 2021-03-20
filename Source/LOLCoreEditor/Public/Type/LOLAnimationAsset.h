#pragma once

#include "CoreMinimal.h"
#include "Type/LOLAsset.h"
#include "Type/LOLAnimation.h"

namespace LOLImporter
{
	struct FLOLAnimationAsset : FLOLAsset
	{
		FLOLAnimation Animation;
		TWeakObjectPtr<USkeleton> Skeleton;

		FLOLAnimationAsset(UObject* InParent, FName InName, EObjectFlags InFlags, TWeakObjectPtr<USkeleton> InSkeleton) : FLOLAsset(InParent, InName, InFlags), Skeleton(InSkeleton){}
	};
}