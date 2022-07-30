#include "ToolProvider.h"
#include "Graphs/Provider/Commands/GraphCommands.h"
#include "Graphs/Provider/Commands/NodeCommands.h"
#include "Kismet/GameplayStatics.h"

UToolProvider::UToolProvider() {
	PrimaryComponentTick.bCanEverTick = true;
}

/*void UToolController::SetSelectionMode(const SelectionMode NewMode) {
	const auto RightHitEntity = VrPawn->GetRightController()->GetHitEntity();
	auto LeftHitEntity = VrPawn->GetLeftController()->GetHitEntity();
	if (RightHitEntity == LeftHitEntity) {
		VrPawn->GetLeftController()->ResetHitResult();
		LeftHitEntity = nullptr;
	}
	if (RightHitEntity != nullptr)
		OnEntityHitChanged(VrPawn->GetRightController(), RightHitEntity, false);
	if (LeftHitEntity != nullptr)
		OnEntityHitChanged(VrPawn->GetLeftController(), LeftHitEntity, false);
	SMode = NewMode;
	if (RightHitEntity != nullptr)
		OnEntityHitChanged(VrPawn->GetRightController(), RightHitEntity, true);
	if (LeftHitEntity != nullptr)
		OnEntityHitChanged(VrPawn->GetLeftController(), LeftHitEntity, true);
}

void UToolController::OnEntityHitChanged(const UVRControllerBase *Controller, AEntity *Entity, const bool IsHit) const {
	if (SMode == SelectionMode::NONE) return;
	SelectionType NewSelection = IsHit ? SelectionType::HIT : SelectionType::NONE;
	if (IsHit)
		Controller->PlayActionHapticEffect();
	if (const auto NodeEntity = Cast<ANodeEntity>(Entity)) {
		const auto OtherController = VrPawn->GetOtherController(Controller);
		if (SMode == SelectionMode::COMPONENT) {
			if (OtherController->GetHitEntity() != Entity) {
				Provider->PushCommand<NodeCommands::SetSelectionType>(NodeEntity, NewSelection);
				Provider->ExecuteQueueCommands();
			}
		}
		else {
			const auto ParentGraph = Entity->GetParentGraph();
			if (OtherController->GetHitEntity() == nullptr || OtherController->GetHitEntity()->GetParentGraph() != ParentGraph) {
				Provider->PushCommand<GraphCommands::SetSelectionType>(ParentGraph, NewSelection);
				Provider->ExecuteQueueCommands();
			}
		}
	}
}*/

void UToolProvider::SetHitResult(const FHitResult& NewHitResult) {
	HitResult = NewHitResult;
	// TODO
	UKismetSystemLibrary::PrintString(GetWorld(), FString::FromInt(HitResult.bBlockingHit));
}

bool UToolProvider::OnRightTriggerAction(const bool IsPressed) {
	// TODO: only for test
	if (HitResult.bBlockingHit) {
		VrPawn->GetRightVrController()->SetToolStateEnabled(IsPressed);
		return true;
	}
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
		/*UGraph *TestGraph = nullptr;
		Provider->PushCommand<GraphCommands::Create>(&TestGraph);
		check(TestGraph == nullptr);
		Provider->ExecuteQueueCommands();
		check(TestGraph != nullptr);
		for (const auto &Pos : Positions)
			Provider->PushCommand<NodeCommands::Create>(TestGraph, Pos);
		Provider->ExecuteQueueCommands();*/
		// for (const auto &Pos : Positions)
		// 	Provider->ExecuteCommand<NodeCommands::Create>(Pos);
	}
}
