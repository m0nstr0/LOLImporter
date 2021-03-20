// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LOLBaseImportOptions.h"
#include "LOLANMImportOptions.generated.h"

UCLASS()
class ULOLANMImportOptions : public ULOLBaseImportOptions
{
	GENERATED_BODY()
	
	virtual void PostInitProperties() override;

public:
	UPROPERTY(EditAnywhere, Category = Animation, meta = (DisplayName = "Skeleton For Animation"))
	USkeleton* SkeletonForAnimation;

	virtual FText GetErrors() override;
};
