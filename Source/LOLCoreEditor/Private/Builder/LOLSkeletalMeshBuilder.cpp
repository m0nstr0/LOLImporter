#include "Builder/LOLSkeletalMeshBuilder.h"
#include "LODUtilities.h"
#include "Util/LOLConverter.h"
#include "ObjectTools.h"
#include "IMeshBuilderModule.h"
#include "Rendering/SkeletalMeshModel.h"
#include "PackageTools.h"

namespace LOLImporter
{
	template<class T>
	T* FLOLSkeletalMeshBuilder::CreateObjectAndPackage(const FLOLSkeletalMeshAsset& Asset,  const FString& AssetNameSufix)
	{
		FString ParentPackageName = UPackageTools::SanitizePackageName(Asset.Parent->GetPathName() + ObjectTools::SanitizeObjectName(AssetNameSufix));
		UObject* ParentPackage = CreatePackage(*ParentPackageName);

		if (ParentPackage == nullptr) {
			return nullptr;
		}

		return NewObject<T>(ParentPackage, *(FPaths::GetBaseFilename(ParentPackageName)), Asset.Flags);
	}

	void FLOLSkeletalMeshBuilder::FillSkeletonData(const FLOLSkeletalMeshAsset& Asset, USkeleton* Skeleton, FReferenceSkeleton& RefSkeleton)
	{
		RefSkeleton.Empty();
		{
			FReferenceSkeletonModifier ReferenceSkeletonModifier(RefSkeleton, Skeleton);

			int16 BoneOffset = 0;
			if (Asset.Skeleton.IsMultiRoot)
			{
				FMeshBoneInfo RootMeshBoneInfo;
				RootMeshBoneInfo.ParentIndex = -1;
				RootMeshBoneInfo.ExportName = TEXT("GENERATED_ROOT");
				RootMeshBoneInfo.Name = FName(TEXT("GENERATED_ROOT"));
				ReferenceSkeletonModifier.Add(RootMeshBoneInfo, FTransform::Identity);
				BoneOffset = 1;
			}

			for (const FLOLJoint& Joint : Asset.Skeleton.Joints)
			{
				FMeshBoneInfo MeshBoneInfo;
				MeshBoneInfo.ParentIndex = Joint.ParentID + BoneOffset;
				MeshBoneInfo.ExportName = Joint.Name + TEXT("_ExportName");
				MeshBoneInfo.Name = *(Joint.Name);

				FTransform Transform;
				Transform.SetLocation(FLOLConverter::ConvertVector(Joint.LocalTranslation));
				Transform.SetRotation(FLOLConverter::ConvertQuat(Joint.LocalRotation));
				Transform.SetScale3D(FLOLConverter::ConvertVector(Joint.LocalScale));

				ReferenceSkeletonModifier.Add(MeshBoneInfo, Transform);
			}
		}
	}

	int32 FLOLSkeletalMeshBuilder::FillMeshMaterialData(const FLOLSubMesh& SubMesh, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh, FLOLSkeletalMeshBuilderRawMapData& RawMapData)
	{
		auto& RawToMaterialMap = RawMapData.RawToMaterialMap;

		if (RawToMaterialMap.Contains(SubMesh.Name)) {
			return RawToMaterialMap[SubMesh.Name];
		}

		FSkeletalMaterial Material;
		Material.ImportedMaterialSlotName = *(SubMesh.Name);
		Material.MaterialSlotName = *(SubMesh.Name);
		Material.MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
		RawToMaterialMap.Add(SubMesh.Name, SkeletalMesh->Materials.Add(Material));
	
		MeshImportData.MaxMaterialIndex = RawToMaterialMap.Num() - 1;

		return RawToMaterialMap[SubMesh.Name];
	}

	bool FLOLSkeletalMeshBuilder::FillSubMeshGeometryData(const FLOLSkeletalMeshAsset& Asset, const FLOLSubMesh& SubMesh, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh, FLOLSkeletalMeshBuilderRawMapData& RawMapData)
	{
		auto& RawToPointMap = RawMapData.RawToPointMap;
		auto& RawToIndexMap = RawMapData.RawToIndexMap;
		const auto& Indices = Asset.Mesh.Indices;
		const auto& Vertices = Asset.Mesh.Vertices;

		for (uint32 IndexID = SubMesh.StartIndex; IndexID < SubMesh.StartIndex + SubMesh.IndexCount; IndexID += 3)
		{
			SkeletalMeshImportData::FTriangle& Triangle = MeshImportData.Faces.Emplace_GetRef();
			Triangle.SmoothingGroups = 255;
			Triangle.MatIndex = FillMeshMaterialData(SubMesh, MeshImportData, SkeletalMesh, RawMapData);

			for (uint32 WeightID = 0; WeightID != 3; WeightID++)
			{
				if (!Indices.IsValidIndex(IndexID + WeightID)) {
					return false;
				}

				const int32 VertexID = Indices[IndexID + WeightID];

				//Add Vertex
				if (!RawToPointMap.Contains(VertexID)) 
				{
					if (!Vertices.IsValidIndex(VertexID)) {
						return false;
					}

					MeshImportData.Points.Add(FLOLConverter::ConvertVector(Vertices[VertexID].Position) * Asset.ImportUniformScale);
					MeshImportData.PointToRawMap.Add(MeshImportData.Points.Num() - 1);

					//Weights & Bones
					for (int32 JointID = 0; JointID < 4; JointID++)
					{
						if (!Asset.Skeleton.JointInfluences.IsValidIndex(Vertices[VertexID].JointIndex[JointID])) {
							return false;
						}

						SkeletalMeshImportData::FRawBoneInfluence& BoneInfluence = MeshImportData.Influences.Emplace_GetRef();
						BoneInfluence.BoneIndex = Asset.Skeleton.JointInfluences[ Vertices[VertexID].JointIndex[JointID] ] + (Asset.Skeleton.IsMultiRoot ? 1 : 0);
						BoneInfluence.VertexIndex = MeshImportData.Points.Num() - 1;
						BoneInfluence.Weight = Vertices[VertexID].Weight[JointID];
					}

					RawToPointMap.Add(VertexID, MeshImportData.Points.Num() - 1);
				}
					
				//Add Index
				if (!RawToIndexMap.Contains(IndexID + WeightID))
				{
					SkeletalMeshImportData::FVertex& Wedge = MeshImportData.Wedges.Emplace_GetRef();
					Wedge.VertexIndex = RawToPointMap[VertexID];
					Wedge.UVs[0] = Vertices[VertexID].TexCoord;

					RawToIndexMap.Add(IndexID + WeightID, MeshImportData.Wedges.Num() - 1);
				}

				Triangle.WedgeIndex[WeightID] = RawToIndexMap[IndexID + WeightID];
				Triangle.TangentX[WeightID] = FVector::ZeroVector;
				Triangle.TangentY[WeightID] = FVector::ZeroVector;
				Triangle.TangentZ[WeightID] = FVector::ZeroVector;
			}
		}
		
		return true;
	}

	bool FLOLSkeletalMeshBuilder::BuildSkeletalMesh(const FLOLSkeletalMeshAsset& Asset, USkeleton* Skeleton, USkeletalMesh* SkeletalMesh, int32 SubMeshID)
	{
		SkeletalMesh->PreEditChange(nullptr);
		SkeletalMesh->InvalidateDeriveDataCacheGUID();

		FSkeletalMeshImportData MeshImportData;
		MeshImportData.bDiffPose = false;
		MeshImportData.bHasNormals = false;
		MeshImportData.bHasTangents = false;
		MeshImportData.bHasVertexColors = false;
		MeshImportData.bUseT0AsRefPose = false;
		MeshImportData.RefBonesBinary.Empty();

		//Positions & Weights
		MeshImportData.Points.Empty(Asset.Mesh.Vertices.Num());
		MeshImportData.PointToRawMap.Empty(Asset.Mesh.Vertices.Num());
		MeshImportData.Influences.Empty(Asset.Mesh.Vertices.Num() * 4);
		MeshImportData.MorphTargetModifiedPoints.Empty();
		MeshImportData.MorphTargetNames.Empty();
		MeshImportData.MorphTargets.Empty();
		MeshImportData.AlternateInfluences.Empty();
		MeshImportData.AlternateInfluenceProfileNames.Empty();
		MeshImportData.Materials.Empty();
		MeshImportData.NumTexCoords = 1;
		MeshImportData.Wedges.Empty(Asset.Mesh.Indices.Num());
		MeshImportData.Faces.Empty(Asset.Mesh.Indices.Num() / 3);
		MeshImportData.MaxMaterialIndex = 0;

		FLOLSkeletalMeshBuilderRawMapData RawMapData;
		const TArray<FLOLSubMesh>& SubMeshes = (SubMeshID == INDEX_NONE) ? Asset.Mesh.SubMeshes : TArray<FLOLSubMesh>({ Asset.Mesh.SubMeshes[SubMeshID] });
		for (const FLOLSubMesh& SubMesh : SubMeshes) {
			if (!FillSubMeshGeometryData(Asset, SubMesh, MeshImportData, SkeletalMesh, RawMapData)) {
				return false;
			}
		}

		FLODUtilities::ProcessImportMeshInfluences(MeshImportData.Wedges.Num(), MeshImportData.Influences, SkeletalMesh->GetPathName());
		SkeletalMesh->SetImportedBounds(FBoxSphereBounds(&MeshImportData.Points[0], (uint32)MeshImportData.Points.Num()));
		SkeletalMesh->ResetLODInfo();

		FSkeletalMeshModel* SkeletalMeshModel = SkeletalMesh->GetImportedModel();
		SkeletalMeshModel->LODModels.Empty();
		SkeletalMeshModel->LODModels.Add(new FSkeletalMeshLODModel());
		FSkeletalMeshLODModel& SkeletalMeshLODModel = SkeletalMeshModel->LODModels[0];
		SkeletalMeshLODModel.NumTexCoords = 1;

		FSkeletalMeshLODInfo& SkeletalMeshLODInfo = SkeletalMesh->AddLODInfo();
		SkeletalMeshLODInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
		SkeletalMeshLODInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
		SkeletalMeshLODInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
		SkeletalMeshLODInfo.LODHysteresis = 0.02f;
		SkeletalMeshLODInfo.BuildSettings.bRecomputeNormals = true;
		SkeletalMeshLODInfo.BuildSettings.bRecomputeTangents = true;
		SkeletalMeshLODInfo.BuildSettings.bBuildAdjacencyBuffer = false;
		SkeletalMeshLODInfo.BuildSettings.bUseMikkTSpace = true;
		SkeletalMeshLODInfo.BuildSettings.bComputeWeightedNormals = true;
		SkeletalMeshLODInfo.BuildSettings.bRemoveDegenerates = false;
		SkeletalMeshLODInfo.BuildSettings.ThresholdPosition = 0.0f;
		SkeletalMeshLODInfo.BuildSettings.ThresholdTangentNormal = 0.0f;
		SkeletalMeshLODInfo.BuildSettings.ThresholdUV = 0.0f;
		SkeletalMeshLODInfo.BuildSettings.MorphThresholdPosition = 0.0f;

		SkeletalMesh->SaveLODImportedData(0, MeshImportData);
		SkeletalMesh->SetLODImportedDataVersions(0, ESkeletalMeshGeoImportVersions::LatestVersion, ESkeletalMeshSkinningImportVersions::LatestVersion);

		FillSkeletonData(Asset, Skeleton, SkeletalMesh->RefSkeleton);

		SkeletalMesh->Skeleton = Skeleton;
		if (!Skeleton->MergeAllBonesToBoneTree(SkeletalMesh) && !Skeleton->RecreateBoneTree(SkeletalMesh)) {
			return nullptr;
		}

		IMeshBuilderModule& MeshBuilderModule = IMeshBuilderModule::GetForRunningPlatform();
		if (!MeshBuilderModule.BuildSkeletalMesh(SkeletalMesh, 0, false)) {
			return false;
		}

		USkeletalMesh::CalculateRequiredBones(SkeletalMesh->GetImportedModel()->LODModels[0], SkeletalMesh->RefSkeleton, nullptr);

		SkeletalMesh->CalculateInvRefMatrices();
		SkeletalMesh->Build();

		return true;
	}

	void FLOLSkeletalMeshBuilder::Clean(TArray<UObject*>& OutAssets)
	{
		for (UObject* Object : OutAssets)
		{
			if (Object != nullptr)
			{
				Object->MarkPendingKill();
			}
		}
		OutAssets.Empty();
	}

	bool FLOLSkeletalMeshBuilder::BuildAssets(const FLOLSkeletalMeshAsset& Asset, TArray<UObject*>& OutAssets)
	{
		OutAssets.Empty();
		USkeleton* Skeleton = CreateObjectAndPackage<USkeleton>(Asset, TEXT("_Skeleton"));
		if (!Skeleton) {
			return false;
		}
		OutAssets.Add(Skeleton);

		if (Asset.SplitMesh)
		{
			for (int32 SubMeshID = 0; SubMeshID < Asset.Mesh.SubMeshes.Num(); SubMeshID++)
			{
				USkeletalMesh* SkeletalMesh = CreateObjectAndPackage<USkeletalMesh>(Asset, TEXT("_") + Asset.Mesh.SubMeshes[SubMeshID].Name);
				if (!SkeletalMesh || !BuildSkeletalMesh(Asset, Skeleton, SkeletalMesh, SubMeshID))
				{
					Clean(OutAssets);
					return false;
				}

				OutAssets.Add(SkeletalMesh);
			}

			return true;
		}
		
		USkeletalMesh* SkeletalMesh = CreateObjectAndPackage<USkeletalMesh>(Asset);
		if (!SkeletalMesh || !BuildSkeletalMesh(Asset, Skeleton, SkeletalMesh))
		{
			Clean(OutAssets);
			return false;
		}

		OutAssets.Add(SkeletalMesh);

		return true;
	}
}