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
			const auto ImporterTool = GetTool<UToolImporter>();
			const auto SelectedItem = Cast<UListItemData>(Item);
			SetPanelType(PanelType::LOADING);

			AsyncTask(
				ENamedThreads::AnyBackgroundHiPriTask,
				[&, ImporterTool, SelectedItem] {
					FString ErrorMessage;
					if (ImporterTool->ImportGraphFromFile(SelectedItem->TextData, ErrorMessage)) {
						SetMessage("Graph has been successfuly\nimported.");
						SetPanelType(PanelType::SUCCESS);
					}
					else {
						SetMessage("Error while importing graph:\n\n" + ErrorMessage);
						SetPanelType(PanelType::ERROR);
					}
				}
			);
		});
	}
}

void UToolImporterPanelWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	static PanelType PrevPanelType = CurrentPanelType;
	if (CurrentPanelType != PrevPanelType && ImporterPanelSwitcher) {
		switch (CurrentPanelType) {
			case PanelType::SUCCESS: {
				if (ImporterText)
					ImporterText->SetText(FText::FromString(Message));
				if (ImporterConfirmButton)
					ImporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
				ImporterPanelSwitcher->SetActiveWidgetIndex(1);
				break;
			}
			case PanelType::ERROR: {
				if (ImporterText)
					ImporterText->SetText(FText::FromString(Message));
				if (ImporterConfirmButton)
					ImporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
				ImporterPanelSwitcher->SetActiveWidgetIndex(1);
				break;
			}
			case PanelType::LOADING: {
				ImporterPanelSwitcher->SetActiveWidgetIndex(2);
				break;
			}
			default: {
				ImporterPanelSwitcher->SetActiveWidgetIndex(0);
			}
		}
		SetCloseToolButtonVisible(CurrentPanelType != PanelType::LOADING);
	}
	PrevPanelType = CurrentPanelType;
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
