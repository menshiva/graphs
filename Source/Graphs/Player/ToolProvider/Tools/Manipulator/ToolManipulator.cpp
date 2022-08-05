#include "ToolManipulator.h"
#include "ToolManipulatorPanelWidget.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"

UToolManipulator::UToolManipulator() : UTool(
	"Manipulate",
	TEXT("/Game/Graphs/UI/Icons/Move"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolManipulatorPanel"),
	{EntityType::GRAPH, EntityType::EDGE, EntityType::VERTEX}
) {}

void UToolManipulator::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
	GetToolPanel<UToolManipulatorPanelWidget>()->SetTextSelectEntity();
}

void UToolManipulator::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

void UToolManipulator::TickTool() {
	Super::TickTool();
	if (GetVrRightController()->IsInToolState()) {
		const auto NewLaserPosition = GetVrRightController()->GetLaserEndPosition();
		const auto Delta = NewLaserPosition - MovePosition;

		switch (GetGraphProvider()->GetConstEntity(GetHitEntityId())->GetType()) {
			case EntityType::VERTEX: {
				GetGraphProvider()->ExecuteCommand<VertexCommands::Move>(GetHitEntityId(), Delta);
				break;
			}
			case EntityType::EDGE: {
				// TODO
				break;
			}
			case EntityType::GRAPH: {
				GetGraphProvider()->ExecuteCommand<GraphCommands::Move>(GetHitEntityId(), Delta);
				break;
			}
		}

		MovePosition = NewLaserPosition;
	}
}

bool UToolManipulator::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed) {
		if (GetHitEntityId() != ENTITY_NONE) {
			MovePosition = GetVrRightController()->GetLaserEndPosition();
			GetToolPanel<UToolManipulatorPanelWidget>()->SetTextMoveEntity();
			GetVrRightController()->SetToolStateEnabled(true);
			GetVrRightController()->SetLaserActive(false);
			return true;
		}
	}
	else {
		GetVrRightController()->SetToolStateEnabled(false);
		GetVrRightController()->SetLaserActive(true);
		GetToolPanel<UToolManipulatorPanelWidget>()->SetTextSelectEntity();
	}
	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolManipulator::OnRightThumbstickY(const float Value) {
	if (GetVrRightController()->IsInToolState())
		GetVrRightController()->SetLaserLengthDelta(Value);
	return Super::OnRightThumbstickY(Value);
}
