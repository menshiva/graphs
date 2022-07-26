#include "ToolController.h"
#include "Graphs/Provider/Commands/NodeCommands.h"
#include "Kismet/GameplayStatics.h"

UToolController::UToolController() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UToolController::OnEntityHitChanged(const UVRControllerBase *ControllerHit, AEntity *Entity, const bool IsHit) const {
	SelectionType NewSelection = IsHit ? SelectionType::HIT : SelectionType::NONE;
	if (IsHit)
		ControllerHit->PlayActionHapticEffect();
	if (const auto NodeEntity = Cast<ANodeEntity>(Entity)) {
		const auto OtherController = VrPawn->GetOtherController(ControllerHit);
		if (OtherController->GetHitEntity() != Entity) {
			Provider->PushCommand<NodeCommands::SetSelectionType>(NodeEntity, NewSelection);
			Provider->MarkDirty();
		}
	}
}

bool UToolController::OnLeftTriggerAction(const bool IsPressed) {
	// TODO: only for test
	if (VrPawn->GetLeftController()->GetHitResult().GetActor() != nullptr) {
		VrPawn->GetLeftController()->SetState(IsPressed ? ControllerState::TOOL : ControllerState::NONE);
		return true;
	}
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftTriggerAction(IsPressed);
}

bool UToolController::OnRightTriggerAction(const bool IsPressed) {
	// TODO: only for test
	if (VrPawn->GetRightController()->GetHitResult().GetActor() != nullptr) {
		VrPawn->GetRightController()->SetState(IsPressed ? ControllerState::TOOL : ControllerState::NONE);
		return true;
	}
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

void UToolController::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// TODO: pass to active tool
}

void UToolController::BeginPlay() {
	Super::BeginPlay();
	Provider = Cast<AGraphProvider>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphProvider::StaticClass()));
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
		for (const auto &Pos : Positions)
			Provider->PushCommand<NodeCommands::Create>(Pos);
		Provider->MarkDirty();
	}
}
