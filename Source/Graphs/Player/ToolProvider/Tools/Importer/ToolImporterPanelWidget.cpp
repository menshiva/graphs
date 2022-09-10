#include "ToolImporterPanelWidget.h"
#include "ToolImporter.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/UI/ListItem/ListItemWidget.h"
#include "Graphs/Utils/Utils.h"

void UToolImporterPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (ImporterRefreshButton)
		ImporterRefreshButton->SetOnClickEvent([&] { GetTool<UToolImporter>()->RefreshFileList(); });
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetOnClickEvent([&] { ShowImportPanel(); });
}

void UToolImporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ImporterList) {
		ImporterList->OnItemClicked().AddLambda([&] (const UObject *Item) {
			const auto SelectedItem = Cast<UListItemData>(Item);
			FString ErrorMessage;
			if (!GetTool<UToolImporter>()->ImportGraphFromFile(SelectedItem->TextData, ErrorMessage))
				ShowErrorPanel(ErrorMessage);
			else
				ShowSuccessPanel();
		});
	}
}

void UToolImporterPanelWidget::SetInputFiles(TArray<FString> &InputFilesPaths) const {
	ImporterList->ClearListItems();
	for (auto &FilePath : InputFilesPaths) {
		const auto NewObj = NewObject<UListItemData>();
		NewObj->TextData = FilePath;

		int32_t Idx;
		const bool Result = FilePath.FindLastChar('/', Idx);
		check(Result);
		FilePath.RemoveAt(0, Idx + 1, false);

		NewObj->DisplayText = FilePath;
		ImporterList->AddItem(NewObj);
	}
}

void UToolImporterPanelWidget::ShowImportPanel() const {
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolImporterPanelWidget::ShowSuccessPanel() const {
	if (ImporterText)
		ImporterText->SetText(FText::FromString("Graph has been successfuly\nimported."));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowErrorPanel(const FString &ErrorMessage) const {
	if (ImporterText)
		ImporterText->SetText(FText::FromString("Error while importing graph:\n\n" + ErrorMessage));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}
