#include "VRControllerRight.h"
#include "NiagaraComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/Pawn/VRPawn.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Right) {
	UiInteractor = ObjectInitializer.CreateDefaultSubobject<UWidgetInteractionComponent>(this, "UiInteractor");
	UiInteractor->PointerIndex = 1;
	UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	UiInteractor->OnHoveredWidgetChanged.AddDynamic(this, &UVRControllerRight::OnUiHover);
	UiInteractor->SetupAttachment(GetMotionControllerAim());
}

void UVRControllerRight::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "RightTrigger", IE_Pressed, [this] {
		OnRightTriggerAction(true);
	});
	BindAction(Pic, "RightTrigger", IE_Released, [this] {
		OnRightTriggerAction(false);
	});
}

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetState() == ControllerState::UI) {
		const auto &hr = UiInteractor->GetLastHitResult();
		if (const auto menu = Cast<UMenuWidgetComponent>(hr.Component.Get()))
			menu->SetCursorLocation(hr.ImpactPoint);
	}
}

bool UVRControllerRight::OnRightTriggerAction(const bool IsPressed) {
	TriggerPressed = IsPressed;
	if (!IsPressed && GetState() == ControllerState::UI && !UiInteractor->IsOverHitTestVisibleWidget()) {
		OnUiHover(nullptr, nullptr);
		return true;
	}
	if (GetState() == ControllerState::UI) {
		if (IsPressed) {
			UiInteractor->PressPointerKey(EKeys::LeftMouseButton);
			PlayActionHapticEffect();
		}
		else {
			UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
		}
		return true;
	}
	if (GetVrPawn()->OnRightTriggerAction(IsPressed)) {
		if (IsPressed)
			PlayActionHapticEffect();
		return true;
	}
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

void UVRControllerRight::SetState(const ControllerState NewState) {
	Super::SetState(NewState);
	if (NewState == ControllerState::NONE && UiInteractor->IsVisible() && UiInteractor->IsOverHitTestVisibleWidget())
		OnUiHover(UiInteractor->GetHoveredWidgetComponent(), nullptr);
}

void UVRControllerRight::SetUiInteractionEnabled(const bool Enabled) {
	if (Enabled) {
		UiInteractor->Activate();
		UiInteractor->SetVisibility(true);
		if (UiInteractor->IsOverHitTestVisibleWidget())
			OnUiHover(UiInteractor->GetHoveredWidgetComponent(), nullptr);
	}
	else {
		UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
		UiInteractor->Deactivate();
		UiInteractor->SetVisibility(false);
		if (GetState() == ControllerState::UI)
			OnUiHover(nullptr, nullptr);
	}
}

void UVRControllerRight::OnUiHover(UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent) {
	if (const auto Menu = Cast<UMenuWidgetComponent>(WidgetComponent)) {
		if (GetState() != ControllerState::TOOL) {
			SetState(ControllerState::UI);
			if (IsLaserVisibleFlag())
				SetLaserActive(false, false);
			ResetHitResult();
			Menu->SetCursorLocation(UiInteractor->GetLastHitResult().ImpactPoint);
			Menu->SetCursorVisibility(true);
		}
	}
	else {
		if (!UiInteractor->IsVisible() || !TriggerPressed) {
			SetState(ControllerState::NONE);
			if (IsLaserVisibleFlag())
				SetLaserActive(true, false);
		}
		if (const auto PrevMenu = Cast<UMenuWidgetComponent>(PreviousWidgetComponent))
			PrevMenu->SetCursorVisibility(false);
	}
}
