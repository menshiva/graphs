#include "ToolManipulatorPanelWidget.h"
#include "Components/TextBlock.h"

void UToolManipulatorPanelWidget::SetTextSelectEntity() const {
	ManipulatorText->SetText(FText::FromString("Select entity with right controller"));
}

void UToolManipulatorPanelWidget::SetTextMoveEntity() const {
	ManipulatorText->SetText(FText::FromString("Move controller or thumbstick up / down to move selected entity"));
}
