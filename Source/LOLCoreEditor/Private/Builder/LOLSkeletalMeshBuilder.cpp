#include "Builder/LOLSkeletalMeshBuilder.h"
#include "LODUtilities.h"
#include "Util/LOLConverter.h"
#include "ObjectTools.h"
#include "IMeshBuilderModule.h"
#include "Rendering/SkeletalMeshModel.h"

namespace LOLImporter
{
	USkeleton* FLOLSkeletalMeshBuilder::CreateSkeleton(const FLOLSkeletalMeshAsset& Asset, USkeletalMesh* SkeletalMesh)
	{
		FString BaseName;
		Asset.Name.ToString(BaseName);
		FName SkeletonName = *(ObjectTools::SanitizeObjectName(FString::Printf(TEXT("%s_Skeleton"), *BaseName)));

		USkeleton* Skeleton = NewObject<USkeleton>(Asset.Parent, SkeletonName, Asset.Flags);

		FReferenceSkeleton ReferenceSkeleton;
		{
			FReferenceSkeletonModifier ReferenceSkeletonModifier(ReferenceSkeleton, Skeleton);
			ReferenceSkeleton.Empty();

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

		Skeleton->RecreateBoneTree(SkeletalMesh);
		SkeletalMesh->Skeleton = Skeleton;
		SkeletalMesh->RefSkeleton = ReferenceSkeleton;

		return Skeleton;
	}

	void FLOLSkeletalMeshBuilder::FillMeshImportData(const FLOLSkeletalMeshAsset& Asset, FSkeletalMeshImportData& MeshImportData, USkeletalMesh* SkeletalMesh)
	{
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
		MeshImportData.MaxMaterialIndex = Asset.Mesh.SubMeshes.Num() - 1;
		MeshImportData.RefBonesBinary.Empty();
		MeshImportData.Materials.Empty();
		MeshImportData.Points.Empty(Asset.Mesh.Vertices.Num());
		MeshImportData.PointToRawMap.Empty(Asset.Mesh.Vertices.Num());
		MeshImportData.Wedges.Empty(Asset.Mesh.Indices.Num());
		MeshImportData.Faces.Empty(Asset.Mesh.Indices.Num() / 3);
		MeshImportData.NumTexCoords = 1;
		MeshImportData.Influences.Empty();

		int16 BoneOffset = 0;
		if (Asset.Skeleton.IsMultiRoot) {
			BoneOffset = 1;
		}

		//Positions & Weights
		for (int32 Idx = 0; Idx < Asset.Mesh.Vertices.Num(); Idx++)
		{
			FLOLVertex Vertex = Asset.Mesh.Vertices[Idx];

			//Positions
			MeshImportData.Points.Add(FLOLConverter::ConvertVector(Vertex.Position));
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

		for (const FLOLSubMesh& SubMesh : Asset.Mesh.SubMeshes)
		{
			FSkeletalMaterial Material;
			Material.ImportedMaterialSlotName = *(SubMesh.Name);
			Material.MaterialSlotName = *(SubMesh.Name);
			Material.MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
			uint32 MaterialIdx = SkeletalMesh->Materials.Add(Material);

			for (uint32 Idx = 0; Idx < SubMesh.IndexCount; Idx += 3) {
				SkeletalMeshImportData::FTriangle Triangle;
				Triangle.SmoothingGroups = 255;
				Triangle.MatIndex = MaterialIdx;

				for (int32 WedgeIdx = 0; WedgeIdx != 3; WedgeIdx++)
				{
					FLOLVertex Vertex = Asset.Mesh.Vertices[Asset.Mesh.Indices[Idx + WedgeIdx + SubMesh.StartIndex]];

					SkeletalMeshImportData::FVertex Wedge;
					Wedge.VertexIndex = Asset.Mesh.Indices[Idx + WedgeIdx + SubMesh.StartIndex];
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
		USkeletalMesh* SkeletalMesh = NewObject<USkeletalMesh>(Asset.Parent, Asset.Name, Asset.Flags);
		SkeletalMesh->PreEditChange(nullptr);
		SkeletalMesh->InvalidateDeriveDataCacheGUID();

		FSkeletalMeshImportData MeshImportData;
		FillMeshImportData(Asset, MeshImportData, SkeletalMesh);

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

		USkeleton* Skeleton = CreateSkeleton(Asset, SkeletalMesh);

		if (!MeshBuilderModule.BuildSkeletalMesh(SkeletalMesh, 0, false))
		{
			return false;
		}

		USkeletalMesh::CalculateRequiredBones(SkeletalMeshLODModel, SkeletalMesh->RefSkeleton, nullptr);

		SkeletalMesh->CalculateInvRefMatrices();
		SkeletalMesh->Build();

		Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
		SkeletalMesh->Skeleton = Skeleton;

		OutAssets.Add(SkeletalMesh);
		OutAssets.Add(Skeleton);

		return true;
	}
}