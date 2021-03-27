#pragma once

#include "CoreMinimal.h"
#include "Type/LOLMesh.h"
#include "Type/LOLSkeleton.h"
#include "Type/LOLAsset.h"
#include "UObject/ObjectMacros.h"

namespace LOLImporter
{
	struct FLOLSkeletalMeshAsset : FLOLAsset
	{
		using FLOLAsset::FLOLAsset;

		FLOLMesh Mesh;
		FLOLSkeleton Skeleton;
		float ImportUniformScale;
		bool SplitMesh;
	};
}