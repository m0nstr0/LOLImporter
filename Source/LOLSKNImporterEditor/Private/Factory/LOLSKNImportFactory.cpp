// Fill out your copyright notice in the Description page of Project Settings.
#include "Factory/LOLSKNImportFactory.h"
#include "Misc/FeedbackContext.h"

#include "UI/LOLImportWindow.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Reader/LOLReaderFactory.h"
#include "Builder/LOLSkeletalMeshBuilder.h"

#define LOCTEXT_NAMESPACE "FLOLSKNImporterEditorModule"

ULOLSKNImportFactory::ULOLSKNImportFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	bText = false;
	SupportedClass = USkeletalMesh::StaticClass();

	Formats.Add(TEXT("skn;League Of Legends Animated Model"));
}

void ULOLSKNImportFactory::PostInitProperties()
{
	Super::PostInitProperties();
	ImportOptions = NewObject<ULOLSKNImportOptions>(this, NAME_None, RF_NoFlags);
}


bool ULOLSKNImportFactory::DoesSupportClass(UClass* Class)
{
	return Class == USkeletalMesh::StaticClass();
}


UObject* ULOLSKNImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
	const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, InName, Parms);
	AdditionalImportedObjects.Empty();
	bOutOperationCanceled = true;

	Warn->Log(Filename);

	if (!LOLImporter::SLOLImportWindow::ShowImportWindow(ImportOptions))
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	LOLImporter::FLOLAsset Asset(InParent, InName, Flags);
	if (!LOLImporter::FLOLReaderFactory::ReadFile(Warn, Asset, Filename) || 
		!LOLImporter::FLOLReaderFactory::ReadFile(Warn, Asset, ImportOptions->SkeletonFilePath.FilePath)) {
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	TArray<UObject*> Objects;
	if (!LOLImporter::FLOLSkeletalMeshBuilder().BuildAssets(Asset, Objects) || Objects.Num() == 0)
	{
		Warn->Log(ELogVerbosity::Error, "Error importing file");
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	for (UObject* Object : Objects)
	{
		if (Object != Objects[0]) {
			AdditionalImportedObjects.Add(Object);
		}

		Object->MarkPackageDirty();
		Object->PostEditChange();
		FAssetRegistryModule::AssetCreated(Object);
	}

	bOutOperationCanceled = false;
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, Objects[0]);
	return Objects[0];
}

UClass* ULOLSKNImportFactory::ResolveSupportedClass()
{
	return USkeletalMesh::StaticClass();
}

#undef LOCTEXT_NAMESPACE