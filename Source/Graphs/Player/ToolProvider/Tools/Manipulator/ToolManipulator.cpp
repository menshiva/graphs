#include "ToolManipulator.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"

UToolManipulator::UToolManipulator() : UTool(
	TEXT("/Game/Graphs/UI/Icons/Move"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolManipulatorPanel"),
	"Manipulate"
) {}

void UToolManipulator::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolManipulator::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetToolStateEnabled(false);
	GetVrRightController()->SetLaserActive(false);
}

void UToolManipulator::TickTool() {
	Super::TickTool();
	if (GetVrRightController()->IsInToolState()) {
		const auto NewLaserPosition = GetVrRightController()->GetLaserEndPosition();
		const auto Delta = NewLaserPosition - MovePosition;

		const auto HitEntity = GetGraphProvider()->GetConstEntity(GetHitEntityId());
		if (HitEntity->GetType() == EntityType::VERTEX) {
			GetGraphProvider()->ExecuteCommand<VertexCommands::Move>(GetHitEntityId(), Delta);
		}
		else if (HitEntity->GetType() == EntityType::EDGE) {
			// TODO
		}
		else if (HitEntity->GetType() == EntityType::GRAPH) {
			GetGraphProvider()->ExecuteCommand<GraphCommands::Move>(GetHitEntityId(), Delta);
		}

		MovePosition = NewLaserPosition;
	}
}

bool UToolManipulator::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed) {
		if (GetHitEntityId() != ENTITY_NONE) {
			MovePosition = GetVrRightController()->GetLaserEndPosition();
			GetVrRightController()->SetToolStateEnabled(true);
			GetVrRightController()->SetLaserActive(false);
			return true;
		}
	}
	else {
		GetVrRightController()->SetToolStateEnabled(false);
		GetVrRightController()->SetLaserActive(true);
	}
	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolManipulator::OnRightThumbstickY(const float Value) {
	if (GetVrRightController()->IsInToolState())
		GetVrRightController()->SetLaserLengthDelta(Value);
	return Super::OnRightThumbstickY(Value);
}
