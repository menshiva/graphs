#include "VRControllerRight.h"
#include "../Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, "Right") {
	m_UiInteractor = CreateDefaultSubobject<UWidgetInteractionComponent>("UiInteractor");
	m_UiInteractor->PointerIndex = 1;
	m_UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	m_UiInteractor->SetAutoActivate(false);
	m_UiInteractor->SetupAttachment(m_MotionControllerAim);
}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) const {
	const auto vrPawn = Cast<AVRPawn>(Pawn);

	PlayerInputComponent->BindAction("Menu", IE_Pressed, vrPawn, &AVRPawn::ToggleMenu);

	AddActionBindingLambda(
		PlayerInputComponent,
		"RightTrigger", IE_Pressed,
		[vrPawn] {
			vrPawn->PrimaryAction(true);
		}
	);
	AddActionBindingLambda(
		PlayerInputComponent,
		"RightTrigger", IE_Released,
		[vrPawn] {
			vrPawn->PrimaryAction(false);
		}
	);
}

void UVRControllerRight::PlayHapticEffect(APlayerController *PlayerController, const float Scale) const {
	PlayerController->PlayHapticEffect(m_ControllerActionHapticEffect, EControllerHand::Right, Scale);
}

void UVRControllerRight::setUiInteractionEnabled(const bool Enabled) const {
	if (Enabled)
		m_UiInteractor->Activate();
	else
		m_UiInteractor->Deactivate();
}

bool UVRControllerRight::IsUiHit() const {
	return m_UiInteractor->IsOverHitTestVisibleWidget();
}

void UVRControllerRight::UiPointerKeyPress() const {
	m_UiInteractor->PressPointerKey(EKeys::LeftMouseButton);
}

void UVRControllerRight::UiPointerKeyRelease() const {
	m_UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
}
