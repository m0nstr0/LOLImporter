#include "UI/LOLImportWindow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Interfaces/IMainFrameModule.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "FLOLImportWindow"

namespace LOLImporter
{
	void SLOLImportWindow::Construct(const FArguments& InArgs)
	{
		ImportOptions = InArgs._ImportOptions;
		Window = InArgs._Window;

		bShouldImport = false;
		TSharedPtr<SBox> DetailsViewBox;

		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0.0f, 10.0f)
			.AutoHeight()
			[
				SNew(SInlineEditableTextBlock)
				.IsReadOnly(true)
				.Text(LOCTEXT("LOLImportFileName", "Import File Name"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SAssignNew(DetailsViewBox, SBox)
				.MinDesiredHeight(320.0f)
				.MinDesiredWidth(450.0f)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(5.f, 0.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("LOLBtnImport", "Import"))
					.OnClicked(this, &SLOLImportWindow::OnImport)
				]
				+ SHorizontalBox::Slot()
				.Padding(5.f, 0.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("LOLBtnCancel", "Cancel"))
					.OnClicked(this, &SLOLImportWindow::OnCancel)
				]
			]
		];

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = true;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

		TSharedPtr<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		DetailsViewBox->SetContent(DetailsView.ToSharedRef());
		DetailsView->SetObject(ImportOptions);
	}

	FReply SLOLImportWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			return OnCancel();
		}

		return FReply::Unhandled();
	}

	FReply SLOLImportWindow::OnImport()
	{
		FText ErrorMsg = ImportOptions->GetErrors();
		bShouldImport = true;

		if (!ErrorMsg.IsEmpty()) {
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMsg);
			bShouldImport = false;
		} 
		else if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}

		return FReply::Handled();
	}

	FReply SLOLImportWindow::OnCancel()
	{
		bShouldImport = false;
		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	bool SLOLImportWindow::ShowImportWindow(ULOLBaseImportOptions* ImportOptions)
	{
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		FVector2D WindowSize = FVector2D(410.0f, 750.0f);
		FSlateRect WorkAreaRect = FSlateApplicationBase::Get().GetPreferredWorkArea();
		FVector2D DisplayTopLeft(WorkAreaRect.Left, WorkAreaRect.Top);
		FVector2D DisplaySize(WorkAreaRect.Right - WorkAreaRect.Left, WorkAreaRect.Bottom - WorkAreaRect.Top);
		float ScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DisplayTopLeft.X, DisplayTopLeft.Y);
		WindowSize *= ScaleFactor;

		FVector2D WindowPosition = (DisplayTopLeft + (DisplaySize - WindowSize) / 2.0f) / ScaleFactor;

		TSharedRef<SWindow> Window = SNew(SWindow)
			.Title(LOCTEXT("LOLImportOptions", "League Of Legends Import Options"))
			.SizingRule(ESizingRule::Autosized)
			.AutoCenter(EAutoCenter::None)
			.ClientSize(WindowSize)
			.ScreenPosition(WindowPosition);


		TSharedPtr<SLOLImportWindow> OptionsWindow;
		Window->SetContent(
			SAssignNew(OptionsWindow, SLOLImportWindow)
			.ImportOptions(ImportOptions)
			.Window(Window)
		);

		FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

		return OptionsWindow->ShouldImport();
	}
}

#undef LOCTEXT_NAMESPACE