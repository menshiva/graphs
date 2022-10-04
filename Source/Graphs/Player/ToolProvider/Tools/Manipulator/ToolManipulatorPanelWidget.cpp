#include "ToolManipulatorPanelWidget.h"
#include "ToolManipulator.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/OptionSelector/OptionSelectorWidget.h"

void UToolManipulatorPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (ModeSelector) {
		ModeSelector->SetOptions({"Move", "Rotate"});
		ModeSelector->SetSelectedOptionIndex(0, false);
	}

	if (RotationModeSelector) {
		RotationModeSelector->SetOptions({"Along Y axis", "Along Z axis"});
		RotationModeSelector->SetSelectedOptionIndex(0, false);
	}
}

void UToolManipulatorPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto ManipulatorTool = GetTool<UToolManipulator>();

	ModeSelector->SetOnSelectedOptionChangedEvent([&, ManipulatorTool] (const int32 SelectedIdx) {
		if (SelectedIdx == 0)
			ManipulatorTool->SetManipulationMode(ManipulationMode::MOVE);
		else if (SelectedIdx == 1)
			ManipulatorTool->SetManipulationMode(ManipulationMode::ROTATE);
		else {
			check(false);
		}
		Update(ManipulatorTool);
	});
	ModeSelector->SetSelectedOptionIndex(0, true);

	RotationModeSelector->SetOnSelectedOptionChangedEvent([&, ManipulatorTool] (const int32 SelectedIdx) {
		if (SelectedIdx == 0)
			ManipulatorTool->SetRotationMode(RotationMode::Y_AXIS);
		else if (SelectedIdx == 1)
			ManipulatorTool->SetRotationMode(RotationMode::Z_AXIS);
		else {
			check(false);
		}
	});
	RotationModeSelector->SetSelectedOptionIndex(0, true);
}

void UToolManipulatorPanelWidget::Update(const UToolManipulator *ManipulatorTool) const {
	const bool IsInMoveMode = ManipulatorTool->GetManipulationMode() == ManipulationMode::MOVE;

	if (ManipulatorTool->GetManipulationEntity() == EntityId::NONE()) {
		ManipulatorText->SetText(
			IsInMoveMode
				? FText::FromString("Select vertex / edge / graph")
				: FText::FromString("Select graph")
		);
	}
	else {
		ManipulatorText->SetText(
			IsInMoveMode
				? FText::FromString("Move controller or thumbstick up / down to move")
				: FText::FromString("Move thumbstick to rotate")
		);
	}

	RotationModeSelector->SetVisibility(
		IsInMoveMode
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible
	);
}
