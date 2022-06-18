#include "VRControllerRight.h"
#include "XRMotionControllerBase.h"
#include "../Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(const FObjectInitializer &ObjectInitializer) : UVRControllerBase(
	ObjectInitializer, FXRMotionControllerBase::RightHandSourceId
) {}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	Super::SetupInputBindings(Pawn, PlayerInputComponent);
	const auto vrPawn = Cast<AVRPawn>(Pawn);
	PlayerInputComponent->BindAxis("RightThumbstickAxisY", vrPawn, &AVRPawn::MoveY);
	PlayerInputComponent->BindAxis("RightThumbstickAxisX", vrPawn, &AVRPawn::MoveX);
	PlayerInputComponent->BindAction("RightTriggerActionPress", IE_Pressed, vrPawn, &AVRPawn::PrimaryAction);
}
