#include "VRControllerRight.h"
#include "SelectionModeSelectorWidget.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Graphs/Utils/Colors.h"

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Right) {
	SetLaserNiagaraColor(ColorUtils::BlueColor);
	SetLaserLength(MeshInteractionLaserMaxDistance);
	UVRControllerBase::SetLaserActive(LaserVisibleFlag);

	UiInteractor = ObjectInitializer.CreateDefaultSubobject<UWidgetInteractionComponent>(this, "UiInteractor");
	UiInteractor->PointerIndex = 1;
	UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	UiInteractor->OnHoveredWidgetChanged.AddDynamic(this, &UVRControllerRight::OnUiHover);
	UiInteractor->SetupAttachment(GetMotionControllerAim());

	SelectionWidgetComponent = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, "SelectionMode");
	const ConstructorHelpers::FClassFinder<UUserWidget> MenuAsset(TEXT("/Game/Graphs/UI/Widgets/SelectionModeSelector"));
	SelectionWidgetComponent->SetWidgetClass(MenuAsset.Class);
	SelectionWidgetComponent->SetDrawAtDesiredSize(true);
	SelectionWidgetComponent->SetPivot({0.5f, 0.5f});
	SelectionWidgetComponent->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, 5.0f),
		FRotator(30.0f, 180.0f, 0.0f)
	);
	SelectionWidgetComponent->SetRelativeScale3D(FVector(0.04f));
	SelectionWidgetComponent->SetGenerateOverlapEvents(false);
	SelectionWidgetComponent->CanCharacterStepUpOn = ECB_No;
	SelectionWidgetComponent->SetCollisionProfileName("NoCollision");
	SelectionWidgetComponent->SetVisibility(false);
	SelectionWidgetComponent->SetupAttachment(GetMotionControllerAim());
}

void UVRControllerRight::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "RightTrigger", IE_Pressed, [this] {
		OnRightTriggerAction(true);
	});
	BindAction(Pic, "RightTrigger", IE_Released, [this] {
		OnRightTriggerAction(false);
	});
	BindAction(Pic, "RightGrip", IE_Pressed, [this] {
		if (State != ControllerState::TOOL) {
			SelectionWidgetComponent->SetVisibility(true);
			PlayActionHapticEffect();
		}
	});
	BindAction(Pic, "RightGrip", IE_Released, [this] {
		if (SelectionWidgetComponent->IsVisible())
			SelectionWidgetComponent->SetVisibility(false);
	});

	BindAxis(Pic, "RightThumbstickAxisY", [this] (const float Value) {
		OnRightThumbstickY(Value);
	});
	BindAxis(Pic, "RightThumbstickAxisX", [this] (const float Value) {
		static bool isClicked = false;
		if (fabsf(Value) >= 0.7f) {
			if (!isClicked) {
				isClicked = true;
				if (Value > 0.0f ? OnRightThumbstickXAction(1.0f) : OnRightThumbstickXAction(-1.0f))
					PlayActionHapticEffect();
			}
		}
		else if (isClicked)
			isClicked = false;
	});
}

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
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

bool UVRControllerRight::OnRightThumbstickY(const float Value) {
	return GetVrPawn()->OnRightThumbstickY(Value);
}

bool UVRControllerRight::OnRightThumbstickXAction(const float Value) {
	if (SelectionWidgetComponent->IsVisible()) {
		check(State != ControllerState::UI);
		SetSelectionMode(Selection == SelectionMode::VERTEX_EDGE ? SelectionMode::GRAPH : SelectionMode::VERTEX_EDGE);
		const auto ToolProvider = GetVrPawn()->GetToolProvider();
		const auto &HitResult = ToolProvider->GetHitResult();
		if (HitResult.bBlockingHit)
			ToolProvider->SetHitResult(HitResult);
		return true;
	}
	return GetVrPawn()->OnRightThumbstickXAction(Value);
}

void UVRControllerRight::SetLaserActive(const bool IsActive) {
	LaserVisibleFlag = IsActive;
	if (State != ControllerState::UI)
		Super::SetLaserActive(IsActive);
	if (State != ControllerState::TOOL)
		GetVrPawn()->GetToolProvider()->ResetHitResult();
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
	static bool IsUiInteractorWasVisible = false;
	if (Enabled) {
		State = ControllerState::TOOL;
		IsUiInteractorWasVisible = UiInteractor->IsVisible();
		if (IsUiInteractorWasVisible)
			SetUiInteractionEnabled(false);
	}
	else {
		State = ControllerState::NONE;
		if (IsUiInteractorWasVisible)
			SetUiInteractionEnabled(true);
		if (UiInteractor->IsVisible() && UiInteractor->IsOverHitTestVisibleWidget())
			OnUiHover(UiInteractor->GetHoveredWidgetComponent(), nullptr);
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

void UVRControllerRight::SetSelectionMode(const SelectionMode NewMode) {
	Selection = NewMode;
	const auto SelectionModeWidget = Cast<USelectionModeSelectorWidget>(SelectionWidgetComponent->GetWidget());
	if (Selection == SelectionMode::VERTEX_EDGE)
		SelectionModeWidget->SetText("Selection Mode: Vertex / Edge");
	else
		SelectionModeWidget->SetText("Selection Mode: Graph");
}
