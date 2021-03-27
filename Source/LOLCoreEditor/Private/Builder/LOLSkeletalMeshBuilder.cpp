#include "Builder/LOLSkeletalMeshBuilder.h"
#include "LODUtilities.h"
#include "Util/LOLConverter.h"
#include "ObjectTools.h"
#include "IMeshBuilderModule.h"
#include "Rendering/SkeletalMeshModel.h"

namespace LOLImporter
{
	USkeleton* FLOLSkeletalMeshBuilder::CreateSkeleton(const FLOLSkeletalMeshAsset& Asset)
	{
		FString BaseName = Asset.Name.ToString();
		FName SkeletonName = *(ObjectTools::SanitizeObjectName(FString::Printf(TEXT("%s_Skeleton"), *BaseName)));

		return NewObject<USkeleton>(Asset.Parent, SkeletonName, Asset.Flags);
	}

	bool FLOLSkeletalMeshBuilder::FillSkeletonData(const FLOLSkeletalMeshAsset& Asset, USkeleton* Skeleton, USkeletalMesh* SkeletalMesh)
	{
		SkeletalMesh->RefSkeleton.Empty();
		{
			FReferenceSkeletonModifier ReferenceSkeletonModifier(SkeletalMesh->RefSkeleton, Skeleton);

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

			for (int32 Idx = 0; Idx < Asset.Skeleton.Joints.Num(); Idx++)
			{
				const FLOLJoint& Joint = Asset.Skeleton.Joints[Idx];
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

		SkeletalMesh->Skeleton = Skeleton;

		if (!Skeleton->MergeAllBonesToBoneTree(SkeletalMesh)) {
			return Skeleton->RecreateBoneTree(SkeletalMesh);
		}

		return true;
	}

	void FLOLSkeletalMeshBuilder::FillMeshImportData(const FLOLSkeletalMeshAsset& Asset, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh, uint32 SubMeshID = 0)
	{
		const int32 VertexCount = Asset.SplitMesh ? Asset.Mesh.SubMeshes[SubMeshID].VertexCount : Asset.Mesh.Vertices.Num();
		const int32 IndexCount = Asset.SplitMesh ? Asset.Mesh.SubMeshes[SubMeshID].IndexCount : Asset.Mesh.Indices.Num();
		const int32 MaxMaterialIndex = Asset.SplitMesh ? 1 : Asset.Mesh.SubMeshes.Num() - 1;
		const int32 StartVertex = Asset.SplitMesh ? Asset.Mesh.SubMeshes[SubMeshID].StartVertex : 0;
		const int32 StartIndex = Asset.SplitMesh ? Asset.Mesh.SubMeshes[SubMeshID].StartIndex : 0;
		const int32 SubMeshCount = Asset.SplitMesh ? 1 : Asset.Mesh.SubMeshes.Num();

		MeshImportData.bDiffPose = false;
		MeshImportData.bHasNormals = false;
		MeshImportData.bHasTangents = false;
		MeshImportData.bHasVertexColors = false;
		MeshImportData.bUseT0AsRefPose = false;
		MeshImportData.MorphTargetModifiedPoints.Empty();
		MeshImportData.MorphTargetNames.Empty();
		MeshImportData.MorphTargets.Empty();
		MeshImportData.AlternateInfluences.Empty();
		MeshImportData.AlternateInfluenceProfileNames.Empty();
		MeshImportData.MaxMaterialIndex = MaxMaterialIndex;
		MeshImportData.RefBonesBinary.Empty();
		MeshImportData.Materials.Empty();
		MeshImportData.Points.Empty(VertexCount);
		MeshImportData.PointToRawMap.Empty(VertexCount);
		MeshImportData.Wedges.Empty(IndexCount);
		MeshImportData.Faces.Empty(IndexCount / 3);
		MeshImportData.NumTexCoords = 1;
		MeshImportData.Influences.Empty();

		int16 BoneOffset = 0;
		if (Asset.Skeleton.IsMultiRoot) {
			BoneOffset = 1;
		}

		//Positions & Weights
		for (int32 Idx = 0; Idx < VertexCount; Idx++)
		{
			FLOLVertex Vertex = Asset.Mesh.Vertices[StartVertex + Idx];

			//Positions
			MeshImportData.Points.Add(FLOLConverter::ConvertVector(Vertex.Position) * Asset.ImportUniformScale);
			MeshImportData.PointToRawMap.Add(Idx);

			//Weights & Bones
			for (int32 BoneIdx = 0; BoneIdx < 4; BoneIdx++)
			{
				SkeletalMeshImportData::FRawBoneInfluence& BoneInfluence = MeshImportData.Influences.Emplace_GetRef();
				BoneInfluence.BoneIndex = Asset.Skeleton.JointInfluences[Vertex.BoneIndex[BoneIdx]] + BoneOffset;
				BoneInfluence.VertexIndex = Idx;
				BoneInfluence.Weight = Vertex.Weight[BoneIdx];
			}
		}

		for (int32 Idx = 0; Idx < SubMeshCount; Idx++)
		{
			const FLOLSubMesh& SubMesh = Asset.Mesh.SubMeshes[(Asset.SplitMesh ? SubMeshID : Idx)];

			FSkeletalMaterial Material;
			Material.ImportedMaterialSlotName = *(SubMesh.Name);
			Material.MaterialSlotName = *(SubMesh.Name);
			Material.MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
			uint32 MaterialIdx = SkeletalMesh->Materials.Add(Material);

			for (uint32 FaceIdx = 0; FaceIdx < SubMesh.IndexCount; FaceIdx += 3) {
				SkeletalMeshImportData::FTriangle Triangle;
				Triangle.SmoothingGroups = 255;
				Triangle.MatIndex = MaterialIdx;

				for (int32 WedgeIdx = 0; WedgeIdx != 3; WedgeIdx++)
				{
					const int32 IndexID = FaceIdx + WedgeIdx + SubMesh.StartIndex;
					FLOLVertex Vertex = Asset.Mesh.Vertices[Asset.Mesh.Indices[IndexID]];

					SkeletalMeshImportData::FVertex Wedge;
					Wedge.VertexIndex = Asset.Mesh.Indices[IndexID] - StartVertex;
					Wedge.UVs[0] = Vertex.TexCoord;
					Triangle.WedgeIndex[WedgeIdx] = MeshImportData.Wedges.Add(Wedge);
					Triangle.TangentX[WedgeIdx] = FVector::ZeroVector;
					Triangle.TangentY[WedgeIdx] = FVector::ZeroVector;
					Triangle.TangentZ[WedgeIdx] = FLOLConverter::ConvertVector(Vertex.Normal);
				}

				MeshImportData.Faces.Add(Triangle);
			}
		}

		FLODUtilities::ProcessImportMeshInfluences(MeshImportData.Wedges.Num(), MeshImportData.Influences, SkeletalMesh->GetPathName());

		SkeletalMesh->SetImportedBounds(FBoxSphereBounds(&MeshImportData.Points[0], (uint32)MeshImportData.Points.Num()));
		SkeletalMesh->ResetLODInfo();
	}

	bool FLOLSkeletalMeshBuilder::BuildAssets(const FLOLSkeletalMeshAsset& Asset, TArray<UObject*>& OutAssets)
	{
		USkeleton* Skeleton = nullptr;
		FReferenceSkeleton ReferenceSkeleton;

		uint32 SubMeshCount = Asset.SplitMesh ? Asset.Mesh.SubMeshes.Num() : 1;
		for (uint32 SubMeshID = 0; SubMeshID != SubMeshCount; SubMeshID++)
		{
			FName AssetName = Asset.Name;
			if (Asset.SplitMesh) {
				AssetName = *(ObjectTools::SanitizeObjectName(AssetName.ToString() + TEXT("_") + Asset.Mesh.SubMeshes[SubMeshID].Name));
			}

			USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(Asset.Parent, AssetName, Asset.Flags);

			if (SkeletalMesh == nullptr) {
				return false;
			}

			SkeletalMesh->PreEditChange(nullptr);
			SkeletalMesh->InvalidateDeriveDataCacheGUID();

			FSkeletalMeshImportData MeshImportData;
			FillMeshImportData(Asset, MeshImportData, SkeletalMesh, SubMeshID);

			IMeshBuilderModule& MeshBuilderModule = IMeshBuilderModule::GetForRunningPlatform();

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

			if (Skeleton == nullptr) {
				Skeleton = CreateSkeleton(Asset);
				if (Skeleton == nullptr) {
					return false;
				}
			}

			FillSkeletonData(Asset, Skeleton, SkeletalMesh);

			if (!MeshBuilderModule.BuildSkeletalMesh(SkeletalMesh, 0, false)) {
				return false;
			}

			USkeletalMesh::CalculateRequiredBones(SkeletalMeshLODModel, SkeletalMesh->RefSkeleton, nullptr);

			SkeletalMesh->CalculateInvRefMatrices();
			SkeletalMesh->Build();

			OutAssets.Add(SkeletalMesh);
		}

		OutAssets.Add(Skeleton);

		return true;
	}
}