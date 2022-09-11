#include "ToolManipulator.h"
#include "ToolManipulatorPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"

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
		SetSupportedEntities({EntitySignature::GRAPH, EntitySignature::EDGE, EntitySignature::VERTEX});
	else
		SetSupportedEntities({EntitySignature::GRAPH});
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

		switch (GetHitEntityId().GetSignature()) {
			case EntitySignature::VERTEX: {
				GetGraphRenderer()->PushCommand(VertexCommands::Move(GetHitEntityId(), Delta));
				break;
			}
			case EntitySignature::EDGE: {
				GetGraphRenderer()->PushCommand(EdgeCommands::Move(GetHitEntityId(), Delta));
				break;
			}
			case EntitySignature::GRAPH: {
				GetGraphRenderer()->PushCommand(GraphCommands::Move(GetHitEntityId(), Delta));
				break;
			}
			default: {
				check(false);
			}
		}

		GetGraphRenderer()->MarkDirty();
		PreviousLaserEndPosition = NewLaserPosition;
	}
}

bool UToolManipulator::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed) {
		if (GetHitEntityId() != EntityId::NONE()) {
			if (Mode == ManipulationMode::MOVE) {
				PreviousLaserEndPosition = GetVrRightController()->GetLaserEndPosition();
			}
			else {
				check(GetHitEntityId().GetSignature() == EntitySignature::GRAPH);
				GraphCenterPosition = GraphCommands::ConstFuncs::ComputeCenterPosition(
					GetEntityStorage(),
					GetHitEntityId()
				);
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
		check(GetHitEntityId().GetSignature() == EntitySignature::GRAPH);
		GetGraphRenderer()->PushCommand(GraphCommands::Rotate(
			GetHitEntityId(),
			GraphCenterPosition,
			Value * DefaultRotationSpeed
		));
		GetGraphRenderer()->MarkDirty();
		return true;
	}
	return Super::OnRightThumbstickX(Value);
}
