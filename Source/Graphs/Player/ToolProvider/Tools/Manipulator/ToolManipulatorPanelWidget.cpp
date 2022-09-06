#include "ToolManipulatorPanelWidget.h"
#include "ToolManipulator.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/OptionSelector/OptionSelectorWidget.h"

void UToolManipulatorPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (ModeSelector) {
		ModeSelector->SetOnSelectedOptionChangedEvent([&] (const int32 SelectedIdx) {
			if (SelectedIdx == 0)
				GetTool<UToolManipulator>()->SetMode(ManipulationMode::MOVE);
			else if (SelectedIdx == 1)
				GetTool<UToolManipulator>()->SetMode(ManipulationMode::ROTATE);
			else {
				check(false);
			}
			SetTextSelectEntity();
		});
		ModeSelector->SetOptions({
			"Mode: Move",
			"Mode: Rotate"
		});
		ModeSelector->SetSelectedOptionIndex(0, true);
	}
}

void UToolManipulatorPanelWidget::SetTextSelectEntity() const {
	if (ManipulatorText) {
		ManipulatorText->SetText(
			GetTool<UToolManipulator>()->GetMode() == ManipulationMode::MOVE
				? FText::FromString("Select vertex, edge or graph")
				: FText::FromString("Select graph")
		);
	}
}

void UToolManipulatorPanelWidget::SetTextActionEntity() const {
	if (ManipulatorText) {
		ManipulatorText->SetText(
			GetTool<UToolManipulator>()->GetMode() == ManipulationMode::MOVE
				? FText::FromString("Move controller or thumbstick up / down to move selected entity")
				: FText::FromString("Move thumbstick left / right to rotate selected entity")
		);
	}
}
