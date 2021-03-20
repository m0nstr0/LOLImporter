#include "Factory/LOLANMImportFactory.h"
#include "Misc/FeedbackContext.h"

#include "UI/LOLImportWindow.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Type/LOLAnimationAsset.h"
#include "Builder/LOLAnimationBuilder.h"
#include "Factory/LOLAnimationReaderFactory.h"

#define LOCTEXT_NAMESPACE "FLOLANMImporterEditorModule"

ULOLANMImportFactory::ULOLANMImportFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	bText = false;
	SupportedClass = UAnimSequence::StaticClass();

	Formats.Add(TEXT("anm;League Of Legends Animation File"));
}

void ULOLANMImportFactory::PostInitProperties()
{
	Super::PostInitProperties();
	ImportOptions = NewObject<ULOLANMImportOptions>(this, NAME_None, RF_NoFlags);
}


bool ULOLANMImportFactory::DoesSupportClass(UClass* Class)
{
	return Class == UAnimSequence::StaticClass();
}


UObject* ULOLANMImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
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
	
	LOLImporter::FLOLAnimationAsset Asset(InParent, InName, Flags, ImportOptions->SkeletonForAnimation);
	if (!LOLImporter::LOLAnimationReaderFactory::Read(Warn, Asset, Filename))
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);
		return nullptr;
	}

	TArray<UObject*> Objects;
	if (!LOLImporter::FLOLAnimationBuilder().BuildAssets(Asset, Objects) || Objects.Num() == 0)
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

UClass* ULOLANMImportFactory::ResolveSupportedClass()
{
	return UAnimSequence::StaticClass();
}

#undef LOCTEXT_NAMESPACE