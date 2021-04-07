// Fill out your copyright notice in the Description page of Project Settings.
#include "Factory/LOLSKNImportFactory.h"
#include "Misc/FeedbackContext.h"

#include "UI/LOLImportWindow.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factory/LOLSkeletalMeshReaderFactory.h"
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

	LOLImporter::FLOLSkeletalMeshAsset Asset(InParent, InName, Flags);
	Asset.SplitMesh = ImportOptions->SplitMesh;
	Asset.ImportUniformScale = ImportOptions->ImportUniformScale;
	Asset.RemoveUnusedJoints = ImportOptions->RemoveUnusedJoints;

	if (!LOLImporter::LOLSkeletalMeshReaderFactory::Read(Warn, Asset, Filename) ||
		!LOLImporter::LOLSkeletalMeshReaderFactory::Read(Warn, Asset, ImportOptions->SkeletonFilePath.FilePath)) 
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	TArray<UObject*> Objects;
	UObject* ObjectToReturn = nullptr;
	if (!LOLImporter::FLOLSkeletalMeshBuilder().BuildAssets(Asset, Objects) || Objects.Num() == 0)
	{
		Warn->Log(ELogVerbosity::Error, "Error importing file");
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	for (UObject* Object : Objects)
	{
		if (ObjectToReturn == nullptr && Object->IsA(USkeletalMesh::StaticClass())) {
			ObjectToReturn = Object;
		} else {
			AdditionalImportedObjects.Add(Object);
		}

		Object->MarkPackageDirty();
		Object->PostEditChange();
		FAssetRegistryModule::AssetCreated(Object);
	}

	bOutOperationCanceled = false;
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, ObjectToReturn);
	return ObjectToReturn;
}

UClass* ULOLSKNImportFactory::ResolveSupportedClass()
{
	return USkeletalMesh::StaticClass();
}

#undef LOCTEXT_NAMESPACE