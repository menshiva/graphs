#include "VRControllerRight.h"
#include "XRMotionControllerBase.h"
#include "../Pawn/VRPlayerPawn.h"

UVRControllerRight::UVRControllerRight(const FObjectInitializer &ObjectInitializer) : UVRControllerBase(
	ObjectInitializer, FXRMotionControllerBase::RightHandSourceId
) {}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	Super::SetupInputBindings(Pawn, PlayerInputComponent);
	const auto vrPawn = Cast<AVRPlayerPawn>(Pawn);
	// TODO
	// PlayerInputComponent->BindAction("RightTriggerActionClick", IE_Pressed, vrPawn, &AVRPlayerPawn::TurnLeft);
	// PlayerInputComponent->BindAction("RightTriggerActionClick", IE_Released, vrPawn, &AVRPlayerPawn::TurnRight);
	PlayerInputComponent->BindAxis("RightThumbstickAxisY", vrPawn, &AVRPlayerPawn::MoveY);
	PlayerInputComponent->BindAxis("RightThumbstickAxisX", vrPawn, &AVRPlayerPawn::MoveX);
}
