#include "VRControllerRight.h"
#include "../Pawn/VRPawn.h"
#include "../Menu/MenuWidgetComponent.h"

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

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaser();
	if (GetState() == ControllerState::UI) {
		const auto &hr = m_UiInteractor->GetLastHitResult();
		if (const auto menu = Cast<UMenuWidgetComponent>(hr.Component.Get()))
			menu->SetCursorLocation(hr.ImpactPoint);
	}
}

void UVRControllerRight::SetUiInteractionEnabled(const bool Enabled) {
	if (Enabled) {
		m_UiInteractor->Activate();
		// fixes cursor when pointing at ui before it becomes visible
		if (const auto menu = Cast<UMenuWidgetComponent>(m_UiInteractor->GetHoveredWidgetComponent()))
			OnUiHover(menu, nullptr);
	}
	else {
		UiLeftMouseButtonRelease();
		m_UiInteractor->Deactivate();
		SetState(ControllerState::NONE);
	}
}

void UVRControllerRight::UiLeftMouseButtonPress() const {
	m_UiInteractor->PressPointerKey(EKeys::LeftMouseButton);
}

void UVRControllerRight::UiLeftMouseButtonRelease() const {
	m_UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
}

void UVRControllerRight::OnUiHover(UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent) {
	if (const auto menu = Cast<UMenuWidgetComponent>(WidgetComponent)) {
		menu->SetCursorLocation(m_UiInteractor->GetLastHitResult().ImpactPoint);
		menu->SetCursorVisibility(true);
		SetState(ControllerState::UI);
	}
	else {
		SetState(ControllerState::NONE);
		if (const auto prevMenu = Cast<UMenuWidgetComponent>(PreviousWidgetComponent))
			prevMenu->SetCursorVisibility(false);
	}
}
