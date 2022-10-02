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
		ImporterConfirmButton->SetOnClickEvent([&] { GetTool<UToolImporter>()->OnAttach(); });
}

void UToolImporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ImporterList) {
		ImporterList->OnItemClicked().AddLambda([&] (const UObject *Item) {
			GetTool<UToolImporter>()->OnImportClick(Cast<UListItemData>(Item)->TextData);
		});
	}
}

void UToolImporterPanelWidget::SetInputFiles(TArray<FString> &&InputFilesPaths) const {
	ImporterList->ClearListItems();
	for (auto &FilePath : InputFilesPaths) {
		const auto NewObj = NewObject<UListItemData>();
		NewObj->TextData = FilePath;

		int32_t Idx;
		const bool Result = FilePath.FindLastChar('/', Idx);
		check(Result);
		FilePath.RemoveAt(0, Idx + 1, false);

		NewObj->DisplayText = MoveTemp(FilePath);
		ImporterList->AddItem(NewObj);
	}
}

void UToolImporterPanelWidget::ShowImportList() const {
	SetCloseToolButtonVisible(true);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolImporterPanelWidget::ShowSuccessPanel() const {
	SetCloseToolButtonVisible(true);
	if (ImporterText)
		ImporterText->SetText(FText::FromString("Graph has been successfuly imported."));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowErrorPanel(const FString &ErrorDesc) const {
	SetCloseToolButtonVisible(true);
	if (ImporterText)
		ImporterText->SetText(FText::FromString(ErrorDesc));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowLoadingPanel() const {
	SetCloseToolButtonVisible(false);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(2);
}
