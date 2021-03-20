#pragma once

#include "CoreMinimal.h"
#include "Type/LOLLogger.h"
#include "Type/LOLSkeletalMeshAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Rendering/SkeletalMeshLODImporterData.h"

namespace LOLImporter
{
	class LOLCOREEDITOR_API FLOLSkeletalMeshBuilder
	{
	private:
		USkeleton* CreateSkeleton(const FLOLSkeletalMeshAsset& Asset, USkeletalMesh* SkeletalMesh);
		void FillMeshImportData(const FLOLSkeletalMeshAsset& Asset, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh);
	public:
		bool BuildAssets(const FLOLSkeletalMeshAsset& Asset, TArray<UObject*>& OutAssets);
	};
}