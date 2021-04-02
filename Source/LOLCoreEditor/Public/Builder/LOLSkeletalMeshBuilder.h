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
		struct FLOLSkeletalMeshBuilderRawMapData
		{
			TMap<int32, int32> RawToPointMap;
			TMap<int32, int32> RawToIndexMap;
			TMap<FString, int32> RawToMaterialMap;
		};
		template<class T>
		T* CreateObjectAndPackage(const FLOLSkeletalMeshAsset& Asset, const FString& AssetNameSufix = "");
		void FillSkeletonData(const FLOLSkeletalMeshAsset& Asset, USkeleton* Skeleton, FReferenceSkeleton& RefSkeleton);
		int32 FillMeshMaterialData(const FLOLSubMesh& SubMesh, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh, FLOLSkeletalMeshBuilderRawMapData& RawMapData);
		bool FillSubMeshGeometryData(const FLOLSkeletalMeshAsset& Asset, const FLOLSubMesh& SubMesh, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh, FLOLSkeletalMeshBuilderRawMapData& RawMapData);
		bool BuildSkeletalMesh(const FLOLSkeletalMeshAsset& Asset, USkeleton* Skeleton, USkeletalMesh* SkeletalMesh, int32 SubMeshID = INDEX_NONE);
		void Clean(TArray<UObject*>& OutAssets);
	public:
		bool BuildAssets(const FLOLSkeletalMeshAsset& Asset, TArray<UObject*>& OutAssets);

	};
}