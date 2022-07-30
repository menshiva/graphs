#include "ToolProvider.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Kismet/GameplayStatics.h"

UToolProvider::UToolProvider() {
	PrimaryComponentTick.bCanEverTick = true;
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
			if (RightController->IsGripPressed()) {
				const auto Entity = GraphProvider->GetConstEntity(Id);
				if (Entity->GetType() == EntityType::VERTEX) {
					const auto Vertex = dynamic_cast<const VertexEntity*>(Entity);
					Id = Vertex->GraphId;
				}
				else if (Entity->GetType() == EntityType::EDGE) {
					// TODO
				}
			}

			HitResult = NewHitResult;
			HitEntityId = Id;
			SetEntitySelectionType(SelectionType::HIT);
			RightController->PlayActionHapticEffect();
		}
	}
}

bool UToolProvider::OnRightTriggerAction(const bool IsPressed) {
	// TODO: only for test
	if (HitResult.bBlockingHit) {
		VrPawn->GetRightVrController()->SetToolStateEnabled(IsPressed);
		return true;
	}
	// if (IsPressed && HitEntityId != ENTITY_NONE) {
	// 	GraphProvider->ExecuteCommand<VertexCommands::Remove>(HitEntityId);
	// 	return true;
	// }
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

void UToolProvider::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// TODO: pass to active tool
}

void UToolProvider::BeginPlay() {
	Super::BeginPlay();
	GraphProvider = Cast<AGraphProvider>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphProvider::StaticClass()));
	// TODO: only for test
	{
		const FVector Positions[] = {
			{437.109619f, 225.096985f, 50.0f},
			{748.974915f, 345.263428f, 260.0f},
			{504.859009f, -437.556763f, 460.0f},
			{969.929321f, -452.031494f, 260.0f},
			{1587.086426f, 611.200684f, 440.0f},
			{1903.230957f, 502.790161f, 650.0f},
			{1213.039551f, 60.030151f, 850.0f},
			{1560.0f, -250.0f, 650.0f},
		};
		EntityId GraphId = ENTITY_NONE;
		GraphProvider->ExecuteCommand<GraphCommands::Create>(&GraphId);
		for (const auto &Pos : Positions)
			GraphProvider->ExecuteCommand<VertexCommands::Create>(GraphId, nullptr, Pos);
	}
}

void UToolProvider::SetEntitySelectionType(const SelectionType Selection) const {
	const auto Entity = GraphProvider->GetConstEntity(HitEntityId);
	if (Entity->Selection != Selection) {
		switch (Entity->GetType()) {
			case EntityType::VERTEX: {
				GraphProvider->ExecuteCommand<VertexCommands::SetSelectionType>(HitEntityId, Selection);
				break;
			}
			case EntityType::EDGE: {
				// TODO
				break;
			}
			case EntityType::GRAPH: {
				GraphProvider->ExecuteCommand<GraphCommands::SetSelectionType>(HitEntityId, Selection);
				break;
			}
		}
	}
}
