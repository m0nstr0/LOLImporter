// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LOLBaseImportOptions.h"
#include "LOLSKNImportOptions.generated.h"

UENUM()
enum class ELOLNormalMapImport : uint8
{
	NM_CALCULATE  UMETA(DisplayName = "Calculate Normals"),
	NM_IMPORT	  UMETA(DisplayName = "Import Normals"),
};

UCLASS()
class ULOLSKNImportOptions : public ULOLBaseImportOptions
{
	GENERATED_BODY()
	
	virtual void PostInitProperties() override;

public:
	UPROPERTY(config, EditAnywhere, Category = SkeletalMesh, meta = (DisplayName = "Skeleton File Path", FilePathFilter = "League Of Legends Skeleton (*.skl)|*.skl"))
	FFilePath SkeletonFilePath;

	UPROPERTY(config, EditAnywhere, Category = Transform)
	float ImportUniformScale;

	UPROPERTY(config, EditAnywhere, Category = Generation)
	bool SplitMesh;

	UPROPERTY(config, EditAnywhere, Category = Generation)
	bool RemoveUnusedJoints;

	virtual FText GetErrors() override;
};
