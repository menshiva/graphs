#include "ToolProvider.h"
#include "Graphs/GraphProvider/Commands/EdgeCommands.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"
#include "Graphs/Utils/Consts.h"
#include "Kismet/GameplayStatics.h"
#include "Tools/Exporter/ToolExporter.h"
#include "Tools/Importer/ToolImporter.h"
#include "Tools/Manipulator/ToolManipulator.h"
#include "Tools/Remover/ToolRemover.h"

UToolProvider::UToolProvider(const FObjectInitializer &ObjectInitializer) : UActorComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	RegisterTool<UToolImporter>(ObjectInitializer, "Tool Importer");
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
		EntityId NewGraphId = ENTITY_NONE;
		FString ErrorMsg;
		GraphProvider->ExecuteCommand(GraphCommands::Import(
			&NewGraphId,
			FPaths::LaunchDir() + FileConsts::ExportDir + "Test_Input\\8_Vertices_8_Edges.json",
			ErrorMsg
		));
		check(NewGraphId != ENTITY_NONE);
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
