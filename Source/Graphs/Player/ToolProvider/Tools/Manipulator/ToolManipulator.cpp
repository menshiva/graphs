#include "ToolManipulator.h"
#include "ToolManipulatorPanelWidget.h"
#include "Graphs/GraphProvider/Commands/EdgeCommands.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"

UToolManipulator::UToolManipulator() : UTool(
	"Manipulate",
	TEXT("/Game/Graphs/UI/Icons/Move"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolManipulatorPanel")
) {
	SetMode(Mode);
}

void UToolManipulator::SetMode(const ManipulationMode NewMode) {
	Mode = NewMode;
	if (Mode == ManipulationMode::MOVE)
		SetSupportedEntityTypes({EntityType::GRAPH, EntityType::EDGE, EntityType::VERTEX});
	else
		SetSupportedEntityTypes({EntityType::GRAPH});
}

void UToolManipulator::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolManipulator::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

void UToolManipulator::TickTool() {
	Super::TickTool();
	if (Mode == ManipulationMode::MOVE && GetVrRightController()->IsInToolState()) {
		const auto NewLaserPosition = GetVrRightController()->GetLaserEndPosition();
		const auto Delta = NewLaserPosition - PreviousLaserEndPosition;

		switch (GetGraphProvider()->GetEntityType(GetHitEntityId())) {
			case EntityType::VERTEX: {
				GetGraphProvider()->ExecuteCommand(VertexCommands::Move(GetHitEntityId(), Delta, true));
				break;
			}
			case EntityType::EDGE: {
				GetGraphProvider()->ExecuteCommand(EdgeCommands::Move(GetHitEntityId(), Delta, true));
				break;
			}
			case EntityType::GRAPH: {
				GetGraphProvider()->ExecuteCommand(GraphCommands::Move(GetHitEntityId(), Delta));
				break;
			}
		}

		PreviousLaserEndPosition = NewLaserPosition;
	}
}

bool UToolManipulator::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed) {
		if (GetHitEntityId() != ENTITY_NONE) {
			if (Mode == ManipulationMode::MOVE) {
				PreviousLaserEndPosition = GetVrRightController()->GetLaserEndPosition();
			}
			else {
				check(GetGraphProvider()->GetEntityType(GetHitEntityId()) == EntityType::GRAPH);
				GetGraphProvider()->ExecuteCommand(GraphCommands::ComputeCenterPosition(
					GetHitEntityId(),
					GraphCenterPosition
				));
			}

			GetToolPanel<UToolManipulatorPanelWidget>()->SetTextActionEntity();
			GetVrRightController()->SetToolStateEnabled(true);
			GetVrRightController()->SetLaserActive(false);
			return true;
		}
	}
	else {
		GetVrRightController()->SetLaserActive(true);
		GetVrRightController()->SetToolStateEnabled(false);
		GetToolPanel<UToolManipulatorPanelWidget>()->SetTextSelectEntity();
	}
	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolManipulator::OnRightThumbstickY(const float Value) {
	if (Mode == ManipulationMode::MOVE && GetVrRightController()->IsInToolState()) {
		GetVrRightController()->SetLaserLengthDelta(Value);
		return true;
	}
	return Super::OnRightThumbstickY(Value);
}

bool UToolManipulator::OnRightThumbstickX(const float Value) {
	if (Mode == ManipulationMode::ROTATE && GetVrRightController()->IsInToolState()) {
		check(GetGraphProvider()->GetEntityType(GetHitEntityId()) == EntityType::GRAPH);
		GetGraphProvider()->ExecuteCommand(GraphCommands::Rotate(
			GetHitEntityId(),
			GraphCenterPosition,
			Value * DefaultRotationSpeed
		));
		return true;
	}
	return Super::OnRightThumbstickX(Value);
}
