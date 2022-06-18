#include "VRControllerLeft.h"
#include "XRMotionControllerBase.h"
#include "../Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(const FObjectInitializer &ObjectInitializer) : UVRControllerBase(
	ObjectInitializer, FXRMotionControllerBase::LeftHandSourceId
) {}

void UVRControllerLeft::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	Super::SetupInputBindings(Pawn, PlayerInputComponent);
	const auto vrPawn = Cast<AVRPawn>(Pawn);
	PlayerInputComponent->BindAction("LeftThumbstickActionLeft", IE_Pressed, vrPawn, &AVRPawn::TurnLeft);
	PlayerInputComponent->BindAction("LeftThumbstickActionRight", IE_Pressed, vrPawn, &AVRPawn::TurnRight);
}
