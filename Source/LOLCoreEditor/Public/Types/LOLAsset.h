#pragma once

#include "CoreMinimal.h"
#include "LOLMesh.h"
#include "LOLSkeleton.h"
#include "UObject/ObjectMacros.h"

namespace LOLImporter
{
	struct FLOLAsset 
	{
		UObject* Parent;
		FName Name;
		EObjectFlags Flags;
		FLOLMesh Mesh;
		FLOLSkeleton Skeleton;

		FLOLAsset(UObject* InParent, FName InName, EObjectFlags InFlags) : Parent(InParent), Name(InName), Flags(InFlags) {}
	};
}