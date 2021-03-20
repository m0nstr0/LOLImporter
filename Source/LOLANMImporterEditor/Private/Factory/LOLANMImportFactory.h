// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "LOLANMImportOptions.h"
#include "LOLANMImportFactory.generated.h"

UCLASS(transient)
class ULOLANMImportFactory : public UFactory
{
	GENERATED_BODY()

	UPROPERTY()
	ULOLANMImportOptions* ImportOptions;

	ULOLANMImportFactory(const FObjectInitializer& ObjectInitializer);

	virtual bool DoesSupportClass(UClass* Class) override;

	virtual void PostInitProperties() override;

public:
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
		const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;

	virtual UClass* ResolveSupportedClass() override;
};