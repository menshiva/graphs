#include "ToolCreatorPanelWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/UI/OptionSelector/OptionSelectorWidget.h"

void UToolCreatorPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (ModeSelector) {
		ModeSelector->SetOptions({"Vertex", "Edge", "Graph"});
		ModeSelector->SetSelectedOptionIndex(0, false);
	}
}

void UToolCreatorPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto CreatorTool = GetTool<UToolCreator>();

	ModeSelector->SetOnSelectedOptionChangedEvent([&, CreatorTool] (const int32 SelectedIdx) {
		switch (SelectedIdx) {
			case 0: {
				CreatorTool->SetMode(CreationMode::VERTEX);
				break;
			}
			case 1: {
				CreatorTool->SetMode(CreationMode::EDGE);
				break;
			}
			case 2: {
				CreatorTool->SetMode(CreationMode::GRAPH);
				break;
			}
			default: {
				check(false);
			}
		}
		Update(CreatorTool);
	});
	ModeSelector->SetSelectedOptionIndex(0, true);

	CancelButton->SetOnClickEvent([CreatorTool] {
		switch (CreatorTool->GetMode()) {
			case CreationMode::VERTEX: {
				CreatorTool->SetGraphSelection(EntityId::NONE());
				break;
			}
			case CreationMode::EDGE: {
				CreatorTool->SetVertexSelection(EntityId::NONE());
				break;
			}
			default: {
				check(false);
			}
		}
	});
}

void UToolCreatorPanelWidget::UpdateModeViaSelector(const CreationMode NewMode) const {
	switch (NewMode) {
		case CreationMode::VERTEX: {
			ModeSelector->SetSelectedOptionIndex(0, true);
			break;
		}
		case CreationMode::EDGE: {
			ModeSelector->SetSelectedOptionIndex(1, true);
			break;
		}
		case CreationMode::GRAPH: {
			ModeSelector->SetSelectedOptionIndex(2, true);
			break;
		}
		default: {
			check(false);
		}
	}
}

void UToolCreatorPanelWidget::Update(const UToolCreator *CreatorTool) const {
	switch (CreatorTool->GetMode()) {
		case CreationMode::VERTEX: {
			if (!CreatorTool->IsGraphSelected()) {
				CancelButton->SetVisibility(ESlateVisibility::Collapsed);
				CreatorText->SetText(FText::FromString("Select a graph"));
			}
			else {
				CancelButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				CreatorText->SetText(FText::FromString("Click to create a new vertex and attach it to the selected graph"));
			}
			break;
		}
		case CreationMode::EDGE: {
			if (!CreatorTool->IsVertexSelected()) {
				CancelButton->SetVisibility(ESlateVisibility::Collapsed);
				CreatorText->SetText(FText::FromString("Select a first vertex"));
			}
			else {
				CancelButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				CreatorText->SetText(FText::FromString("Select a second vertex to create a new edge"));
			}
			break;
		}
		case CreationMode::GRAPH: {
			CancelButton->SetVisibility(ESlateVisibility::Collapsed);
			CreatorText->SetText(FText::FromString("Click to create a new graph with 1 vertex"));
			break;
		}
		default: {
			check(false);
		}
	}
}
