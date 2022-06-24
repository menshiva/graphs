#include "VRControllerRight.h"
#include "../Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, "Right") {}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) const {
	const auto vrPawn = Cast<AVRPawn>(Pawn);

	AddActionBindingLambda(
		PlayerInputComponent,
		"RightTriggerActionPress", IE_Pressed,
		[vrPawn] {
			vrPawn->PrimaryAction(true);
		}
	);
	AddActionBindingLambda(
		PlayerInputComponent,
		"RightTriggerActionPress", IE_Released,
		[vrPawn] {
			vrPawn->PrimaryAction(false);
		}
	);
}

void UVRControllerRight::PlayHapticEffect(APlayerController *PlayerController, const float Scale) const {
	PlayerController->PlayHapticEffect(m_ControllerActionHapticEffect, EControllerHand::Right, Scale);
}
