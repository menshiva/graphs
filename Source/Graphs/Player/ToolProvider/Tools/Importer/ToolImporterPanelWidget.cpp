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

	ImporterList->OnItemClicked().AddLambda([ImportTool] (const UObject *Item) {
		ImportTool->ImportFromFile(Cast<UListItemData>(Item)->TextData);
	});

	ImporterRefreshButton->SetOnClickEvent([&] {
		check(ImporterPanelSwitcher->GetActiveWidgetIndex() == 0);
		if (!RefreshList())
			ShowErrorPanel(TEXT("Failed to create export directory."));
	});

	ImporterConfirmButton->SetOnClickEvent([&, ImportTool] {
		check(ImporterPanelSwitcher->GetActiveWidgetIndex() == 1);
		ImportTool->DeselectImportedGraph();
		if (RefreshList())
			ShowImportList();
		else
			ShowErrorPanel(TEXT("Failed to create export directory."));
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
	ImporterText->SetText(FText::FromString("Graph has been successfuly imported."));
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
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto ExportDirPath = FPaths::LaunchDir() + FileConsts::ExportDirName;

	if (!FileManager.CreateDirectoryTree(*ExportDirPath)) {
		ShowErrorPanel(TEXT("Failed to create export directory."));
		return false;
	}

	TArray<FString> ImportFiles;
	FileManager.FindFiles(ImportFiles, *ExportDirPath, TEXT(".json"));

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
