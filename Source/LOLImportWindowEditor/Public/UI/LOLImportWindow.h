#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "LOLBaseImportOptions.h"

namespace LOLImporter
{
	class LOLIMPORTWINDOWEDITOR_API SLOLImportWindow : public SCompoundWidget
	{
	private:
		ULOLBaseImportOptions* ImportOptions;
		bool bShouldImport;
		TWeakPtr<SWindow> Window;
	public:
		SLATE_BEGIN_ARGS(SLOLImportWindow) {}
			SLATE_ARGUMENT(ULOLBaseImportOptions*, ImportOptions);
			SLATE_ARGUMENT(TWeakPtr<SWindow>, Window);
		SLATE_END_ARGS();

		FReply OnImport();
		FReply OnCancel();

		void Construct(const FArguments& InArgs);
		virtual bool SupportsKeyboardFocus() const override { return true; }
		virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
		bool ShouldImport() const { return bShouldImport; }

		static bool ShowImportWindow(ULOLBaseImportOptions* ImportOptions);
	};
}