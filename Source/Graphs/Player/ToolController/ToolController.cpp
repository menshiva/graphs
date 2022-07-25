#include "ToolController.h"
#include "Graphs/Provider/Commands/NodeCommands.h"
#include "Kismet/GameplayStatics.h"

UToolController::UToolController() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UToolController::OnEntityHitChanged(const UVRControllerBase *ControllerHit, const AEntity* Entity, const bool IsHit) const {
	// TODO: do it only if active tool can handle this entity
	if (const auto PrimitiveComponent = Cast<UPrimitiveComponent>(Entity->GetStaticMeshComponent())) {
		if (IsHit)
			ControllerHit->PlayActionHapticEffect();
		UVRControllerBase *OtherController;
		if (ControllerHit == VrPawn->GetRightController())
			OtherController = VrPawn->GetLeftController();
		else
			OtherController = VrPawn->GetRightController();
		if (OtherController->GetHitResult().GetActor() != Entity)
			PrimitiveComponent->SetRenderCustomDepth(IsHit);
	}
}

bool UToolController::OnLeftTriggerAction(const bool IsPressed) {
	// TODO: do it only if active tool can handle this entity
	if (VrPawn->GetLeftController()->GetHitResult().GetActor() != nullptr) {
		VrPawn->GetLeftController()->SetState(IsPressed ? ControllerState::TOOL : ControllerState::NONE);
		return true;
	}
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftTriggerAction(IsPressed);
}

bool UToolController::OnRightTriggerAction(const bool IsPressed) {
	// TODO: do it only if active tool can handle this entity
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
			Provider->PushCommand<NodeCommands::Create>(nullptr, Pos);
		Provider->MarkDirty();
	}
}
