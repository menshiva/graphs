#include "VRControllerRight.h"
#include "../Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Right) {}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	const auto vrPawn = Cast<AVRPawn>(Pawn);
	PlayerInputComponent->BindAction("RightTriggerActionPress", IE_Pressed, vrPawn, &AVRPawn::PrimaryActionPressed);
	PlayerInputComponent->BindAction("RightTriggerActionPress", IE_Released, vrPawn, &AVRPawn::PrimaryActionReleased);
}
