#include "VRControllerLeft.h"
#include "XRMotionControllerBase.h"
#include "../Pawn/VRPlayerPawn.h"

UVRControllerLeft::UVRControllerLeft(const FObjectInitializer &ObjectInitializer) : UVRControllerBase(
	ObjectInitializer, FXRMotionControllerBase::LeftHandSourceId
) {}

void UVRControllerLeft::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	Super::SetupInputBindings(Pawn, PlayerInputComponent);
	const auto vrPawn = Cast<AVRPlayerPawn>(Pawn);
	PlayerInputComponent->BindAction("LeftThumbstickActionLeft", IE_Pressed, vrPawn, &AVRPlayerPawn::TurnLeft);
	PlayerInputComponent->BindAction("LeftThumbstickActionRight", IE_Pressed, vrPawn, &AVRPlayerPawn::TurnRight);
}
