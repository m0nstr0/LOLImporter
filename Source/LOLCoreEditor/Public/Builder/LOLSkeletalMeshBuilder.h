#pragma once

#include "CoreMinimal.h"
#include "Types/LOLLogger.h"
#include "Types/LOLAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Rendering/SkeletalMeshLODImporterData.h"

namespace LOLImporter
{
	class LOLCOREEDITOR_API FLOLSkeletalMeshBuilder
	{
	public:
		USkeleton* CreateSkeleton(const FLOLAsset& Asset, USkeletalMesh* SkeletalMesh);
		void FillMeshImportData(const FLOLAsset& Asset, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh);
		bool BuildAssets(const FLOLAsset& Asset, TArray<UObject*>& OutAssets);
	};
}