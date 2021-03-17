// Fill out your copyright notice in the Description page of Project Settings.


#include "LOLSKNImportOptions.h"

void ULOLSKNImportOptions::PostInitProperties()
{
	Super::PostInitProperties();

	NormalImportMethod = ELOLNormalMapImport::NM_CALCULATE;
	ImportUniformScale = 1.0f;
	ComputeWeighedNormals = true;
	GenerateLightmapUV = true;
}


FText ULOLSKNImportOptions::GetErrors()
{
	if (SkeletonFilePath.FilePath.IsEmpty()) {
		return FText::FromString("Please, select an appropriate skeleton file for this skin");
	}

	return FText::GetEmpty();
}