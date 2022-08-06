#include "ToolManipulatorPanelWidget.h"
#include "ToolManipulator.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void UToolManipulatorPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ModeSlider)
		ModeSlider->OnValueChanged.AddDynamic(this, &UToolManipulatorPanelWidget::OnSliderValueChanged);
}

void UToolManipulatorPanelWidget::SetTextSelectEntity() const {
	ManipulatorText->SetText(
		GetTool<UToolManipulator>()->GetMode() == ManipulationMode::MOVE
			? FText::FromString("Select vertex, edge or graph")
			: FText::FromString("Select graph")
	);
}

void UToolManipulatorPanelWidget::SetTextActionEntity() const {
	ManipulatorText->SetText(
		GetTool<UToolManipulator>()->GetMode() == ManipulationMode::MOVE
			? FText::FromString("Move controller or thumbstick up / down to move selected entity")
			: FText::FromString("Move thumbstick left / right to rotate selected entity")
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UToolManipulatorPanelWidget::OnSliderValueChanged(const float Value) {
	GetTool<UToolManipulator>()->SetMode(Value == 0.0f ? ManipulationMode::MOVE : ManipulationMode::ROTATE);
	SetTextSelectEntity();
}
