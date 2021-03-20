// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LOLBaseImportOptions.generated.h"

UCLASS(BlueprintType, config = EditorPerProjectUserSettings, HideCategories = (DebugProperty))
class LOLIMPORTWINDOWEDITOR_API ULOLBaseImportOptions : public UObject
{
	GENERATED_BODY()
	
public:
	virtual FText GetErrors();
};
