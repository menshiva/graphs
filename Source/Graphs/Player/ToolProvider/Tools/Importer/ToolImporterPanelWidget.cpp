#include "ToolImporterPanelWidget.h"
#include "ToolImporter.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/UI/ListItem/ListItemWidget.h"
#include "Graphs/Utils/Utils.h"

void UToolImporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto ImportTool = GetTool<UToolImporter>();

	// widgets callback bindings

	ImporterList->OnItemClicked().AddLambda([ImportTool] (const UObject *Item) {
		ImportTool->ImportFromFile(CastChecked<UListItemData>(Item)->TextData);
	});

	ImporterRefreshButton->SetOnClickEvent([&] {
		check(ImporterPanelSwitcher->GetActiveWidgetIndex() == 0);
		if (!RefreshList())
			ShowErrorPanel(TEXT("Failed to create the Export directory."));
	});

	ImporterConfirmButton->SetOnClickEvent([&, ImportTool] {
		check(ImporterPanelSwitcher->GetActiveWidgetIndex() == 1);
		ImportTool->DeselectImportedGraph();
		ShowImportList();
	});
}

void UToolImporterPanelWidget::ShowImportList() const {
	SetCloseToolButtonVisible(true);
	if (RefreshList())
		ImporterPanelSwitcher->SetActiveWidgetIndex(0);
	else
		ShowErrorPanel(TEXT("Failed to create export directory."));
}

void UToolImporterPanelWidget::ShowSuccessPanel() const {
	SetCloseToolButtonVisible(true);
	ImporterText->SetText(FText::FromString("The graph has been successfuly imported."));
	ImporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
	ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowErrorPanel(const FStringView &ErrorDesc) const {
	SetCloseToolButtonVisible(true);
	ImporterText->SetText(FText::FromString(ErrorDesc.GetData()));
	ImporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
	ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowLoadingPanel() const {
	SetCloseToolButtonVisible(false);
	ImporterPanelSwitcher->SetActiveWidgetIndex(2);
}

bool UToolImporterPanelWidget::RefreshList() const {
	TArray<FString> ImportFiles;
	if (!UToolImporter::GetExportFolderContents(ImportFiles))
		return false;

	ImporterList->ClearListItems();
	for (auto &FilePath : ImportFiles) {
		const auto NewObj = NewObject<UListItemData>();
		NewObj->TextData = FilePath;

		// remove trailing slash
		int32_t Idx;
		const bool Result = FilePath.FindLastChar('/', Idx);
		check(Result);
		FilePath.RemoveAt(0, Idx + 1, false);

		NewObj->DisplayText = MoveTemp(FilePath);
		ImporterList->AddItem(NewObj);
	}

	return true;
}
