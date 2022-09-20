#include "ToolManipulator.h"
#include "ToolManipulatorPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolManipulator::TickTool"), STAT_UToolManipulator_TickTool, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightTriggerAction"), STAT_UToolManipulator_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightThumbstickY"), STAT_UToolManipulator_OnRightThumbstickY, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightThumbstickX"), STAT_UToolManipulator_OnRightThumbstickX, STATGROUP_GRAPHS_PERF);

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
		SetSupportedEntities({GRAPH, EDGE, VERTEX});
	else
		SetSupportedEntities({GRAPH});
}

void UToolManipulator::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolManipulator::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
	ManipulationEntity = EntityId::NONE();
}

void UToolManipulator::TickTool() {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_TickTool);
	Super::TickTool();
	if (Mode == ManipulationMode::MOVE && GetVrRightController()->IsInToolState()) {
		const auto NewLaserPosition = GetVrRightController()->GetLaserEndPosition();
		const auto Delta = NewLaserPosition - PreviousLaserEndPosition;

		if (!Delta.IsNearlyZero(0.1f)) {
			if (ES::IsValid<VertexEntity>(ManipulationEntity)) {
				GetGraphsRenderer()->ExecuteCommand(VertexCommands::Move(ManipulationEntity, Delta), true);
			}
			else if (ES::IsValid<EdgeEntity>(ManipulationEntity)) {
				GetGraphsRenderer()->ExecuteCommand(EdgeCommands::Move(ManipulationEntity, Delta), true);
			}
			else {
				check(ES::IsValid<GraphEntity>(ManipulationEntity));
				GetGraphsRenderer()->ExecuteCommand(GraphCommands::Move(ManipulationEntity, Delta), true);
			}
		}

		PreviousLaserEndPosition = NewLaserPosition;
	}
}

bool UToolManipulator::OnRightTriggerAction(const bool IsPressed) {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_OnRightTriggerAction);

	if (IsPressed) {
		if (GetHitEntityId() != EntityId::NONE()) {
			ManipulationEntity = GetHitEntityId();

			if (Mode == ManipulationMode::MOVE) {
				PreviousLaserEndPosition = GetVrRightController()->GetLaserEndPosition();
			}
			else {
				check(ES::IsValid<GraphEntity>(ManipulationEntity));
				GraphCenterPosition = GraphCommands::ConstFuncs::ComputeCenterPosition(ManipulationEntity);
				LastThumbstickXValue = 0.0f;
			}

			GetToolProvider()->ExecuteHitCommandBasedOnHitEntity(false);
			GetGraphsRenderer()->MarkDirty();

			GetToolPanel<UToolManipulatorPanelWidget>()->SetTextActionEntity();
			GetVrRightController()->SetToolStateEnabled(true);
			GetVrRightController()->SetLaserActive(false);
			return true;
		}
	}
	else if (ManipulationEntity != EntityId::NONE()) {
		ManipulationEntity = EntityId::NONE();

		GetToolProvider()->ExecuteHitCommandBasedOnHitEntity(true);
		GetGraphsRenderer()->ExecuteCommand(GraphCommands::UpdateCollisions(), true);

		GetVrRightController()->SetLaserActive(true);
		GetVrRightController()->SetToolStateEnabled(false);
		GetToolPanel<UToolManipulatorPanelWidget>()->SetTextSelectEntity();
	}

	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolManipulator::OnRightThumbstickY(const float Value) {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_OnRightThumbstickY);

	if (Mode == ManipulationMode::MOVE && GetVrRightController()->IsInToolState()) {
		GetVrRightController()->SetLaserLengthDelta(Value);
		return true;
	}

	return Super::OnRightThumbstickY(Value);
}

bool UToolManipulator::OnRightThumbstickX(const float Value) {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_OnRightThumbstickX);

	if (Value != LastThumbstickXValue || Value > 0.0f) {
		if (Mode == ManipulationMode::ROTATE && GetVrRightController()->IsInToolState()) {
			check(ES::IsValid<GraphEntity>(ManipulationEntity));
			GetGraphsRenderer()->ExecuteCommand(GraphCommands::Rotate(
				ManipulationEntity,
				GraphCenterPosition,
				Value * DefaultRotationSpeed
			), true);
			return true;
		}
	}

	LastThumbstickXValue = Value;
	return Super::OnRightThumbstickX(Value);
}
