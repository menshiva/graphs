#include "ToolProvider.h"
#include "Graphs/GraphProvider/Commands/EdgeCommands.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/Exporter/ToolExporter.h"
#include "Tools/Manipulator/ToolManipulator.h"
#include "Tools/Remover/ToolRemover.h"

UToolProvider::UToolProvider(const FObjectInitializer &ObjectInitializer) : UActorComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	RegisterTool<UToolExporter>(ObjectInitializer, "Tool Exporter");
	RegisterTool<UToolManipulator>(ObjectInitializer, "Tool Manipulator");
	RegisterTool<UToolRemover>(ObjectInitializer, "Tool Remover");
}

void UToolProvider::SetHitResult(const FHitResult &NewHitResult) {
	if (HitEntityId != ENTITY_NONE && GraphProvider->IsEntityValid(HitEntityId))
		SetEntitySelectionType(SelectionType::NONE);
	HitResult.Reset();
	HitEntityId = ENTITY_NONE;
	if (NewHitResult.bBlockingHit && NewHitResult.GetActor()) {
		EntityId Id = NewHitResult.GetActor()->GetUniqueID();
		if (GraphProvider->IsEntityValid(Id)) {
			const auto RightController = VrPawn->GetRightVrController();

			if (RightController->GetSelectionMode() == SelectionMode::GRAPH) {
				const auto EntityType = GraphProvider->GetEntityType(Id);
				check(EntityType == EntityType::VERTEX || EntityType == EntityType::EDGE);
				EntityId GraphId = ENTITY_NONE;
				if (EntityType == EntityType::VERTEX)
					GetGraphProvider()->ExecuteCommand(VertexCommands::GetGraphId(Id, GraphId));
				else
					GetGraphProvider()->ExecuteCommand(EdgeCommands::GetGraphId(Id, GraphId));
				check(GetGraphProvider()->IsEntityValid(GraphId));
				Id = GraphId;
			}

			if (!ActiveTool.IsValid() || ActiveTool->SupportsType(GraphProvider->GetEntityType(Id))) {
				HitResult = NewHitResult;
				HitEntityId = Id;
				SetEntitySelectionType(SelectionType::HIT);
			}
		}
	}
}

bool UToolProvider::OnRightTriggerAction(const bool IsPressed) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightTriggerAction(IsPressed);
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

bool UToolProvider::OnRightThumbstickY(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickY(Value);
	return RightControllerInputInterface::OnRightThumbstickY(Value);
}

bool UToolProvider::OnRightThumbstickX(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickX(Value);
	return RightControllerInputInterface::OnRightThumbstickX(Value);
}

bool UToolProvider::OnRightThumbstickXAction(const float Value) {
	if (ActiveTool.IsValid())
		return ActiveTool->OnRightThumbstickXAction(Value);
	return RightControllerInputInterface::OnRightThumbstickXAction(Value);
}

void UToolProvider::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (ActiveTool.IsValid())
		ActiveTool->TickTool();
}

void UToolProvider::SetActiveTool(UTool *NewTool) {
	if (ActiveTool.IsValid())
		ActiveTool->OnDetach();
	ActiveTool = NewTool;
	if (ActiveTool.IsValid())
		ActiveTool->OnAttach();
}

void UToolProvider::BeginPlay() {
	Super::BeginPlay();
	GraphProvider = Cast<AGraphProvider>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphProvider::StaticClass()));
	// TODO: only for test
	{
		const TArray<FVector> Positions = {
			{437.109619f, 225.096985f, 50.0f},
			{748.974915f, 345.263428f, 260.0f},
			{504.859009f, -437.556763f, 460.0f},
			{969.929321f, -452.031494f, 260.0f},
			{1587.086426f, 611.200684f, 440.0f},
			{1903.230957f, 502.790161f, 650.0f},
			{1213.039551f, 60.030151f, 850.0f},
			{1560.0f, -250.0f, 650.0f},
		};
		const TArray<std::pair<uint32_t, uint32_t>> Connections = {
			{0, 1},
			{0, 2},
			{0, 3},
			{1, 2},
			{1, 4},
			{4, 5},
			{5, 6},
			{6, 4},
		};
		TMap<uint32_t, EntityId> VertexDisplayIdToEntityId;

		EntityId GraphId = ENTITY_NONE;
		GraphProvider->ExecuteCommand(GraphCommands::Create(&GraphId));
		for (size_t i = 0; i < Positions.Num(); ++i) {
			EntityId NewVertexId = ENTITY_NONE;
			GraphProvider->ExecuteCommand(VertexCommands::Create(GraphId, &NewVertexId, i, Positions[i]));
			VertexDisplayIdToEntityId.Add(i, NewVertexId);
		}
		for (size_t i = 0; i < Connections.Num(); ++i) {
			const EntityId FirstVertexId = VertexDisplayIdToEntityId.FindChecked(Connections[i].first);
			const EntityId SecondVertexId = VertexDisplayIdToEntityId.FindChecked(Connections[i].second);
			GraphProvider->ExecuteCommand(EdgeCommands::Create(
				GraphId,
				FirstVertexId, SecondVertexId,
				nullptr
			));
		}
	}
}

void UToolProvider::SetEntitySelectionType(const SelectionType Selection) const {
	const auto HitEntityType = GraphProvider->GetEntityType(HitEntityId);

	if (HitEntityType == EntityType::VERTEX)
		GraphProvider->ExecuteCommand(VertexCommands::SetSelectionType(HitEntityId, Selection));
	else if (HitEntityType == EntityType::EDGE)
		GraphProvider->ExecuteCommand(EdgeCommands::SetSelectionType(HitEntityId, Selection));
	else {
		check(HitEntityType == EntityType::GRAPH);
		GraphProvider->ExecuteCommand(GraphCommands::SetSelectionType(HitEntityId, Selection));
	}
}
