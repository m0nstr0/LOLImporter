#pragma once

#include "CoreMinimal.h"
#include "LOLMesh.h"
#include "LOLSkeleton.h"
#include "LOLAnimation.h"
#include "UObject/ObjectMacros.h"

namespace LOLImporter
{
	struct FLOLAsset 
	{
		UObject* Parent;
		FName Name;
		EObjectFlags Flags;

		FLOLAsset(UObject* InParent, FName InName, EObjectFlags InFlags) : Parent(InParent), Name(InName), Flags(InFlags) {}
	};
}