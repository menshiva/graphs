#include "ToolManipulate.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"

void UToolManipulate::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolManipulate::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetToolStateEnabled(false);
	GetVrRightController()->SetLaserActive(false);
}

void UToolManipulate::TickTool() {
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

bool UToolManipulate::OnRightTriggerAction(const bool IsPressed) {
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

bool UToolManipulate::OnRightThumbstickY(const float Value) {
	const auto RController = GetVrRightController();
	if (RController->IsInToolState())
		RController->SetLaserLengthDelta(Value);
	return Super::OnRightThumbstickY(Value);
}
