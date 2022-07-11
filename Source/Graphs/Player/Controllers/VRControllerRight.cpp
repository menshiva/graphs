#include "VRControllerRight.h"
#include "../Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : USceneComponent(ObjectInitializer), UVRControllerBase(ObjectInitializer, this, "Right") {
	PrimaryComponentTick.bCanEverTick = true;

	m_UiInteractor = ObjectInitializer.CreateDefaultSubobject<UWidgetInteractionComponent>(this, "UiInteractor");
	m_UiInteractor->PointerIndex = 1;
	m_UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	m_UiInteractor->SetAutoActivate(false);
	m_UiInteractor->OnHoveredWidgetChanged.AddDynamic(this, &UVRControllerRight::OnUiHover);
	m_UiInteractor->SetupAttachment(GetMotionControllerAim());
}

void UVRControllerRight::SetupInputBindings(APawn *Pawn, UInputComponent *Pic) const {
	const auto vrPawn = Cast<AVRPawn>(Pawn);
	BindAction(Pic, "RightTrigger", IE_Pressed, [vrPawn] {
		vrPawn->OnRightTriggerPressed();
	});
	BindAction(Pic, "RightTrigger", IE_Released, [vrPawn] {
		vrPawn->OnRightTriggerReleased();
	});
}

void UVRControllerRight::PlayHapticEffect(APlayerController *PlayerController, const float Scale) {
	PlayerController->PlayHapticEffect(GetHapticEffectController(), EControllerHand::Right, Scale);
}

void UVRControllerRight::SetState(const ControllerState NewState) {
	UVRControllerBase::SetState(NewState);
	if (NewState != ControllerState::UI) {
		SetLaserActive(true);
	}
	else {
		SetLaserActive(false);
	}
}

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaser();
}

void UVRControllerRight::SetUiInteractionEnabled(const bool Enabled) const {
	Enabled ? m_UiInteractor->Activate() : m_UiInteractor->Deactivate();
}

void UVRControllerRight::UiLeftMouseButtonPress() const {
	m_UiInteractor->PressPointerKey(EKeys::LeftMouseButton);
}

void UVRControllerRight::UiLeftMouseButtonRelease() const {
	m_UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
}

void UVRControllerRight::OnUiHover(
	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	UWidgetComponent *WidgetComponent,
	[[maybe_unused]] UWidgetComponent *PreviousWidgetComponent
) {
	if (WidgetComponent == nullptr)
		SetState(ControllerState::NONE);
	else
		SetState(ControllerState::UI);
}
