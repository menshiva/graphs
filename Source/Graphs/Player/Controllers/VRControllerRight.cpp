#include "VRControllerRight.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/UI/MenuWidgetComponent.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : USceneComponent(ObjectInitializer), UVRControllerBase(ObjectInitializer, this, EControllerHand::Right) {
	PrimaryComponentTick.bCanEverTick = true;

	UiInteractor = ObjectInitializer.CreateDefaultSubobject<UWidgetInteractionComponent>(this, "UiInteractor");
	UiInteractor->PointerIndex = 1;
	UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	UiInteractor->SetAutoActivate(false);
	UiInteractor->OnHoveredWidgetChanged.AddDynamic(this, &UVRControllerRight::OnUiHover);
	UiInteractor->Deactivate();
	UiInteractor->SetVisibility(false);
	UiInteractor->SetupAttachment(MotionControllerAim.Get());
}

void UVRControllerRight::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "RightTrigger", IE_Pressed, [this] {
		OnRightTriggerAction(true);
	});
	BindAction(Pic, "RightTrigger", IE_Released, [this] {
		OnRightTriggerAction(false);
	});

	BindAxis(Pic, "RightThumbstickAxisY", [this] (const float Value) {
		OnRightThumbstickYAxis(Value);
	});
	BindAxis(Pic, "RightThumbstickAxisX", [this] (const float Value) {
		OnRightThumbstickXAxis(Value);
	});
}

void UVRControllerRight::SetState(const ControllerState NewState) {
	OnRightStateChanged(NewState);
	UVRControllerBase::SetState(NewState);
}

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaser();
	if (GetState() == ControllerState::UI) {
		const auto &hr = UiInteractor->GetLastHitResult();
		if (const auto menu = Cast<UMenuWidgetComponent>(hr.Component.Get()))
			menu->SetCursorLocation(hr.ImpactPoint);
	}
}

void UVRControllerRight::SetUiInteractionEnabled(const bool Enabled) {
	if (Enabled) {
		UiInteractor->Activate();
		UiInteractor->SetVisibility(true);
		// fixes cursor when pointing at ui before it becomes visible
		if (const auto menu = Cast<UMenuWidgetComponent>(UiInteractor->GetHoveredWidgetComponent()))
			OnUiHover(menu, nullptr);
	}
	else {
		SetUiInteractorPointerKeyPressed(false, EKeys::LeftMouseButton);
		if (GetState() != ControllerState::TOOL)
			SetState(ControllerState::NONE);
		UiInteractor->Deactivate();
		UiInteractor->SetVisibility(false);
	}
}

void UVRControllerRight::SetUiInteractorPointerKeyPressed(const bool IsPressed, const FKey &Key) const {
	if (IsPressed)
		UiInteractor->PressPointerKey(Key);
	else
		UiInteractor->ReleasePointerKey(Key);
}

void UVRControllerRight::OnRightStateChanged(const ControllerState NewState) {
	RightControllerInputInterface::OnRightStateChanged(NewState);
	SetLaserActive(NewState != ControllerState::UI);
	VrPawn->OnRightStateChanged(NewState);
}

bool UVRControllerRight::OnRightTriggerAction(const bool IsPressed) {
	TriggerPressed = IsPressed;
	if (VrPawn->OnRightTriggerAction(IsPressed)) {
		if (IsPressed)
			PlayActionHapticEffect();
		return true;
	}
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

bool UVRControllerRight::OnRightThumbstickYAction(const float Value) {
	if (VrPawn->OnRightThumbstickYAction(Value)) {
		PlayActionHapticEffect();
		return true;
	}
	return RightControllerInputInterface::OnRightThumbstickYAction(Value);
}

bool UVRControllerRight::OnRightThumbstickXAction(const float Value) {
	if (VrPawn->OnRightThumbstickXAction(Value)) {
		PlayActionHapticEffect();
		return true;
	}
	return RightControllerInputInterface::OnRightThumbstickXAction(Value);
}

bool UVRControllerRight::OnRightThumbstickYAxis(const float Value) {
	ThumbstickY = Value;
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked) {
			isClicked = true;
			if (Value > 0.0f) OnRightThumbstickYAction(1.0f);
			else OnRightThumbstickYAction(-1.0f);
		}
	}
	else if (isClicked)
		isClicked = false;
	return VrPawn->OnRightThumbstickYAxis(Value);
}

bool UVRControllerRight::OnRightThumbstickXAxis(const float Value) {
	ThumbstickX = Value;
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked) {
			isClicked = true;
			if (Value > 0.0f) OnRightThumbstickXAction(1.0f);
			else OnRightThumbstickXAction(-1.0f);
		}
	}
	else if (isClicked)
		isClicked = false;
	return VrPawn->OnRightThumbstickXAxis(Value);
}

void UVRControllerRight::OnUiHover(UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent) {
	if (GetState() != ControllerState::TOOL) {
		if (const auto menu = Cast<UMenuWidgetComponent>(WidgetComponent)) {
			SetState(ControllerState::UI);
			menu->SetCursorLocation(UiInteractor->GetLastHitResult().ImpactPoint);
			menu->SetCursorVisibility(true);
		}
		else {
			SetState(ControllerState::NONE);
			if (const auto prevMenu = Cast<UMenuWidgetComponent>(PreviousWidgetComponent))
				prevMenu->SetCursorVisibility(false);
		}
	}
}
