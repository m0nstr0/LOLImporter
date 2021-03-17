// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "LOLSKNImportOptions.h"
#include "LOLSKNImportFactory.generated.h"

UCLASS(transient)
class ULOLSKNImportFactory : public UFactory
{
	GENERATED_BODY()

	UPROPERTY()
	ULOLSKNImportOptions* ImportOptions;

	ULOLSKNImportFactory(const FObjectInitializer& ObjectInitializer);

	virtual bool DoesSupportClass(UClass* Class) override;

	virtual void PostInitProperties() override;

public:
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
		const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	virtual UClass* ResolveSupportedClass() override;
};