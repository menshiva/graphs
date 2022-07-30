#include "EditorTool.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"

void UEditorTool::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UEditorTool::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

void UEditorTool::TickTool() {
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

bool UEditorTool::OnRightTriggerAction(const bool IsPressed) {
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

bool UEditorTool::OnRightThumbstickY(const float Value) {
	const auto RController = GetVrRightController();
	if (RController->IsInToolState())
		RController->SetLaserLengthDelta(Value);
	return Super::OnRightThumbstickY(Value);
}
