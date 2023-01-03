#include "ToolManipulator.h"
#include "ToolManipulatorPanelWidget.h"
#include "Graphs/EntityStorage/Commands/EdgeCommands.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"
#include "Graphs/EntityStorage/Commands/VertexCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolManipulator::TickTool"), STAT_UToolManipulator_TickTool, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightTriggerAction"), STAT_UToolManipulator_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightThumbstickY"), STAT_UToolManipulator_OnRightThumbstickY, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolManipulator::OnRightThumbstickX"), STAT_UToolManipulator_OnRightThumbstickX, STATGROUP_GRAPHS_PERF);

UToolManipulator::UToolManipulator() : UTool(
	"Manipulate",
	TEXT("/Game/Graphs/UI/Icons/Move"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolManipulatorPanel")
) {}

void UToolManipulator::OnAttach() {
	Super::OnAttach();
	SetManipulationMode(ManipMode);
}

void UToolManipulator::OnDetach() {
	Super::OnDetach();
	check(ManipulationEntity == EntityId::NONE());
}

void UToolManipulator::TickTool() {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_TickTool);
	Super::TickTool();
	if (ManipMode == ManipulationMode::MOVE && GetVrRightController()->IsInToolState()) {
		const auto NewLaserPosition = GetVrRightController()->GetLaserEndPosition();
		const auto Delta = NewLaserPosition - PreviousLaserEndPosition;

		if (!Delta.IsNearlyZero(0.1f)) {
			if (ES::IsValid<VertexEntity>(ManipulationEntity)) {
				VertexCommands::Mutable::Move(ManipulationEntity, Delta);
				GetGraphsRenderers()->MarkVertexDirty(ManipulationEntity, true, false, true);
				GetGraphsRenderers()->RedrawChunkByVertexIfDirty(ManipulationEntity, true);
			}
			else if (ES::IsValid<EdgeEntity>(ManipulationEntity)) {
				EdgeCommands::Mutable::Move(ManipulationEntity, Delta);
				GetGraphsRenderers()->MarkEdgeDirty(ManipulationEntity, true, false, true);
				GetGraphsRenderers()->RedrawChunkByEdgeIfDirty(ManipulationEntity, true);
			}
			else {
				GraphCommands::Mutable::Move(ManipulationEntity, Delta);
				GetGraphsRenderers()->MarkGraphDirty(
					ManipulationEntity,
					true, false,
					true, false
				);
				GetGraphsRenderers()->RedrawGraphChunksIfDirty(ManipulationEntity);
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

			if (ManipMode == ManipulationMode::MOVE) {
				PreviousLaserEndPosition = GetVrRightController()->GetLaserEndPosition();
			}
			else {
				check(ES::IsValid<GraphEntity>(ManipulationEntity));
				GraphCenterPosition = GraphCommands::Const::ComputeCenterPosition(ManipulationEntity);
				LastThumbstickXValue = 0.0f;
			}

			GetToolProvider()->ExecuteHitCommandBasedOnHitEntity(false);

			GetToolPanel<UToolManipulatorPanelWidget>()->Update(this);
			GetVrRightController()->SetToolStateEnabled(true);
			GetVrRightController()->SetLaserActive(false);
			return true;
		}
	}
	else if (ManipulationEntity != EntityId::NONE()) {
		GetToolProvider()->ExecuteHitCommandBasedOnHitEntity(true);

		// update collisions
		if (ES::IsValid<VertexEntity>(ManipulationEntity)) {
			GetGraphsRenderers()->MarkVertexDirty(ManipulationEntity, false, true, true);
			GetGraphsRenderers()->RedrawChunkByVertexIfDirty(ManipulationEntity, true);
		}
		else if (ES::IsValid<EdgeEntity>(ManipulationEntity)) {
			GetGraphsRenderers()->MarkEdgeDirty(ManipulationEntity, false, true, true);
			GetGraphsRenderers()->RedrawChunkByEdgeIfDirty(ManipulationEntity, true);
		}
		else {
			check(ES::IsValid<GraphEntity>(ManipulationEntity));
			GetGraphsRenderers()->MarkGraphDirty(
				ManipulationEntity,
				false, true,
				false, true
			);
			GetGraphsRenderers()->RedrawGraphChunksIfDirty(ManipulationEntity);
		}

		ManipulationEntity = EntityId::NONE();

		GetVrRightController()->SetLaserActive(true);
		GetVrRightController()->SetToolStateEnabled(false);
		GetToolPanel<UToolManipulatorPanelWidget>()->Update(this);
	}

	return Super::OnRightTriggerAction(IsPressed);
}

bool UToolManipulator::OnRightThumbstickY(const float Value) {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_OnRightThumbstickY);

	if (GetVrRightController()->IsInToolState()) {
		if (ManipMode == ManipulationMode::MOVE) {
			GetVrRightController()->SetLaserLengthDelta(Value);
			LastThumbstickYValue = Value;
			return true;
		}
		check(ManipMode == ManipulationMode::ROTATE);
		if (Value != LastThumbstickYValue && fabs(Value) > fabs(LastThumbstickXValue)) {
			check(ES::IsValid<GraphEntity>(ManipulationEntity));
			GraphCommands::Mutable::Rotate(
				ManipulationEntity,
				GraphCenterPosition,
				FVector::RightVector,
				Value * DefaultRotationSpeed
			);
			GetGraphsRenderers()->MarkGraphDirty(
				ManipulationEntity,
				true, false,
				true, false
			);
			GetGraphsRenderers()->RedrawGraphChunksIfDirty(ManipulationEntity);
			LastThumbstickYValue = Value;
			return true;
		}
	}

	LastThumbstickYValue = Value;
	return Super::OnRightThumbstickY(Value);
}

bool UToolManipulator::OnRightThumbstickX(const float Value) {
	SCOPE_CYCLE_COUNTER(STAT_UToolManipulator_OnRightThumbstickX);

	if (ManipMode == ManipulationMode::ROTATE && GetVrRightController()->IsInToolState()) {
		if (Value != LastThumbstickXValue && fabs(Value) > fabs(LastThumbstickYValue)) {
			check(ES::IsValid<GraphEntity>(ManipulationEntity));
			GraphCommands::Mutable::Rotate(
				ManipulationEntity,
				GraphCenterPosition,
				FVector::DownVector,
				Value * DefaultRotationSpeed
			);
			GetGraphsRenderers()->MarkGraphDirty(
				ManipulationEntity,
				true, false,
				true, false
			);
			GetGraphsRenderers()->RedrawGraphChunksIfDirty(ManipulationEntity);
			LastThumbstickXValue = Value;
			return true;
		}
	}

	LastThumbstickXValue = Value;
	return Super::OnRightThumbstickX(Value);
}

void UToolManipulator::SetManipulationMode(const ManipulationMode NewMode) {
	ManipMode = NewMode;
	if (ManipMode == ManipulationMode::MOVE) {
		SetSupportedEntities({GRAPH, EDGE, VERTEX});
	}
	else {
		check(ManipMode == ManipulationMode::ROTATE);
		SetSupportedEntities({GRAPH});
	}
}
