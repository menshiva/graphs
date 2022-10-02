#include "ToolCreatorPanelWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/UI/OptionSelector/OptionSelectorWidget.h"

void UToolCreatorPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	const auto CreatorTool = GetTool<UToolCreator>();
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

void UToolCreatorPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ModeSelector) {
		ModeSelector->SetOnSelectedOptionChangedEvent([&] (const int32 SelectedIdx) {
			switch (SelectedIdx) {
				case 0: {
					GetTool<UToolCreator>()->SetMode(CreationMode::VERTEX);
					break;
				}
				case 1: {
					GetTool<UToolCreator>()->SetMode(CreationMode::EDGE);
					break;
				}
				case 2: {
					GetTool<UToolCreator>()->SetMode(CreationMode::GRAPH);
					break;
				}
				default: {
					check(false);
				}
			}
			Update();
		});
		ModeSelector->SetOptions({
			"Vertex",
			"Edge",
			"Graph"
		});
		ModeSelector->SetSelectedOptionIndex(0, true);
	}
}

void UToolCreatorPanelWidget::UpdateModeViaSelector(const CreationMode NewMode) const {
	if (ModeSelector) {
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
}

void UToolCreatorPanelWidget::Update() const {
	if (CreatorText && CancelButton) {
		const auto CreatorTool = GetTool<UToolCreator>();
		switch (CreatorTool->GetMode()) {
			case CreationMode::VERTEX: {
				if (!CreatorTool->IsGraphSelected()) {
					CancelButton->SetVisibility(ESlateVisibility::Collapsed);
					CreatorText->SetText(FText::FromString("Select graph"));
				}
				else {
					CancelButton->SetVisibility(ESlateVisibility::Visible);
					CreatorText->SetText(FText::FromString("Click to create a new vertex and attach it to selected graph"));
				}
				break;
			}
			case CreationMode::EDGE: {
				if (!CreatorTool->IsVertexSelected()) {
					CancelButton->SetVisibility(ESlateVisibility::Collapsed);
					CreatorText->SetText(FText::FromString("Select first vertex"));
				}
				else {
					CancelButton->SetVisibility(ESlateVisibility::Visible);
					CreatorText->SetText(FText::FromString("Select second vertex to create a new edge"));
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
}
