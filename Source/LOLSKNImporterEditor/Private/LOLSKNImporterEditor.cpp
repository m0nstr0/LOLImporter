// Copyright Epic Games, Inc. All Rights Reserved.

#include "LOLSKNImporterEditor.h"
//#include "LOLImportOptionsCustomization.h"
//#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FLOLSKNImporterEditorModule"

void FLOLSKNImporterEditorModule::StartupModule()
{
//	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");
//	PropertyModule.RegisterCustomClassLayout("LOLImportOptions", FOnGetDetailCustomizationInstance::CreateStatic(&FLOLImportOptionsCustomization::MakeInstance));
//	PropertyModule.NotifyCustomizationModuleChanged();
}

void FLOLSKNImporterEditorModule::ShutdownModule()
{
//	FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
//	if (PropertyModule) {
//		PropertyModule->UnregisterCustomClassLayout("LOLImportOptions");
//		PropertyModule->NotifyCustomizationModuleChanged();
//	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLOLSKNImporterEditorModule, LOLSKNImporterEditor)