#include "VRControllerRight.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Graphs/Utils/Colors.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Right) {
	SetLaserNiagaraColor(ColorUtils::SelectionColor);
	SetLaserLength(MeshInteractionLaserMaxDistance);
	UVRControllerBase::SetLaserActive(LaserVisibleFlag);

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
	if (State == ControllerState::UI) {
		const auto &UiHitResult = UiInteractor->GetLastHitResult();
		if (const auto Menu = Cast<UMenuWidgetComponent>(UiHitResult.Component.Get()))
			Menu->SetCursorLocation(UiHitResult.ImpactPoint);
	}
	else if (State == ControllerState::NONE && IsLaserActive()) {
		FHitResult NewHitResult;
		GetWorld()->LineTraceSingleByChannel(
			NewHitResult,
			GetLaserStartPosition(),
			GetLaserStartPosition() + MeshInteractionLaserMaxDistance * GetLaserDirection(),
			ECC_GameTraceChannel2 // Graph trace channel
		);
		const auto ToolProvider = GetVrPawn()->GetToolProvider();
		if (NewHitResult.GetActor() != ToolProvider->GetHitResult().GetActor())
			ToolProvider->SetHitResult(NewHitResult);
		if (NewHitResult.bBlockingHit)
			SetLaserLength(FVector::Dist(GetLaserStartPosition(), NewHitResult.ImpactPoint));
		else
			SetLaserLength(MeshInteractionLaserMaxDistance);
	}
}

bool UVRControllerRight::OnRightTriggerAction(const bool IsPressed) {
	TriggerPressed = IsPressed;
	if (!IsPressed && State == ControllerState::UI && !UiInteractor->IsOverHitTestVisibleWidget()) {
		OnUiHover(nullptr, nullptr);
		return true;
	}
	if (State == ControllerState::UI) {
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

void UVRControllerRight::SetLaserActive(const bool IsActive) {
	LaserVisibleFlag = IsActive;
	if (State != ControllerState::UI) {
		Super::SetLaserActive(IsActive);
		GetVrPawn()->GetToolProvider()->ResetHitResult();
	}
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
		if (State == ControllerState::UI)
			OnUiHover(nullptr, nullptr);
	}
}

void UVRControllerRight::SetToolStateEnabled(const bool Enabled) {
	if (Enabled) {
		State = ControllerState::TOOL;
		UVRControllerBase::SetLaserActive(false);
	}
	else {
		State = ControllerState::NONE;
		if (UiInteractor->IsVisible() && UiInteractor->IsOverHitTestVisibleWidget())
			OnUiHover(UiInteractor->GetHoveredWidgetComponent(), nullptr);
		if (State == ControllerState::NONE && LaserVisibleFlag)
			UVRControllerBase::SetLaserActive(true);
	}
}

void UVRControllerRight::OnUiHover(UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent) {
	if (const auto Menu = Cast<UMenuWidgetComponent>(WidgetComponent)) {
		if (State != ControllerState::TOOL) {
			State = ControllerState::UI;
			if (LaserVisibleFlag) {
				UVRControllerBase::SetLaserActive(false);
				GetVrPawn()->GetToolProvider()->ResetHitResult();
			}
			Menu->SetCursorLocation(UiInteractor->GetLastHitResult().ImpactPoint);
			Menu->SetCursorVisibility(true);
		}
	}
	else {
		if (!UiInteractor->IsVisible() || !TriggerPressed) {
			State = ControllerState::NONE;
			if (LaserVisibleFlag)
				UVRControllerBase::SetLaserActive(true);
		}
		if (const auto PrevMenu = Cast<UMenuWidgetComponent>(PreviousWidgetComponent))
			PrevMenu->SetCursorVisibility(false);
	}
}
