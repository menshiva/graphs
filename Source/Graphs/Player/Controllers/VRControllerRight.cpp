#include "VRControllerRight.h"
#include "Components/WidgetInteractionComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Graphs/UI/OptionSelector/OptionSelectorWidget.h"

DECLARE_CYCLE_STAT(TEXT("UVRControllerRight::Tick"), STAT_UVRControllerRight_Tick, STATGROUP_GRAPHS_PERF);

UVRControllerRight::UVRControllerRight(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Right) {
	SetLaserNiagaraColor(ColorConsts::BlueColor.ReinterpretAsLinear());
	SetLaserLength(MeshInteractionLaserMaxDistance);
	UVRControllerBase::SetLaserActive(LaserVisibleFlag);

	UiInteractor = ObjectInitializer.CreateDefaultSubobject<UWidgetInteractionComponent>(this, "UiInteractor");
	UiInteractor->PointerIndex = 1;
	UiInteractor->TraceChannel = ECC_GameTraceChannel1; // VRUI trace channel
	UiInteractor->OnHoveredWidgetChanged.AddDynamic(this, &UVRControllerRight::OnUiHover);
	UiInteractor->SetupAttachment(GetMotionControllerAim());

	SelectionWidgetComponent = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, "SelectionMode");
	const ConstructorHelpers::FClassFinder<UOptionSelectorWidget> SelectorAsset(TEXT("/Game/Graphs/UI/Widgets/OptionSelector"));
	SelectionWidgetComponent->SetWidgetClass(SelectorAsset.Class);
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
		if (OnRightTriggerAction(true))
			PlayActionHapticEffect();
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
		OnRightThumbstickX(Value);
	});
}

void UVRControllerRight::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	SCOPE_CYCLE_COUNTER(STAT_UVRControllerRight_Tick);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State == ControllerState::UI) {
		const auto &UiHitResult = UiInteractor->GetLastHitResult();
		if (const auto Menu = Cast<UMenuWidgetComponent>(UiHitResult.Component.Get()))
			Menu->SetCursorLocation(UiHitResult.ImpactPoint);
	}
	else if (State == ControllerState::NONE && IsLaserActive()) {
		FHitResult NewHitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bReturnFaceIndex = true;
		QueryParams.bTraceComplex = false;
		QueryParams.MobilityType = EQueryMobilityType::Static;
		GetWorld()->LineTraceSingleByChannel(
			NewHitResult,
			GetLaserStartPosition(),
			GetLaserStartPosition() + MeshInteractionLaserMaxDistance * GetLaserDirection(),
			ECC_GameTraceChannel2, // Graph trace channel
			QueryParams
		);
		GetVrPawn()->GetToolProvider()->SetHitResult(NewHitResult);
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
		if (IsPressed)
			UiInteractor->PressPointerKey(EKeys::LeftMouseButton);
		else
			UiInteractor->ReleasePointerKey(EKeys::LeftMouseButton);
		return true;
	}
	return GetVrPawn()->OnRightTriggerAction(IsPressed);
}

bool UVRControllerRight::OnRightThumbstickY(const float Value) {
	if (State == ControllerState::UI && Value != 0.0f) {
		UiInteractor->ScrollWheel(Value * 0.25f);
		return true;
	}
	return GetVrPawn()->OnRightThumbstickY(Value);
}

bool UVRControllerRight::OnRightThumbstickX(const float Value) {
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
	return GetVrPawn()->OnRightThumbstickX(Value);
}

bool UVRControllerRight::OnRightThumbstickXAction(const float Value) {
	if (SelectionWidgetComponent->IsVisible()) {
		check(State != ControllerState::TOOL);
		const auto SelectorWidget = Cast<UOptionSelectorWidget>(SelectionWidgetComponent->GetWidget());
		if (Value < 0.0f)
			SelectorWidget->SelectPreviousOption(true);
		else if (Value > 0.0f)
			SelectorWidget->SelectNextOption(true);
		else {
			check(false);
		}
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
		if (SelectionWidgetComponent->IsVisible())
			SelectionWidgetComponent->SetVisibility(false);
	}
	else {
		State = ControllerState::NONE;
		if (IsUiInteractorWasVisible)
			SetUiInteractionEnabled(true);
		if (UiInteractor->IsVisible() && UiInteractor->IsOverHitTestVisibleWidget())
			OnUiHover(UiInteractor->GetHoveredWidgetComponent(), nullptr);
	}
}

void UVRControllerRight::BeginPlay() {
	Super::BeginPlay();

	if (!SelectionWidgetComponent->GetWidget())
		SelectionWidgetComponent->InitWidget();

	const auto SelectorWidget = Cast<UOptionSelectorWidget>(SelectionWidgetComponent->GetWidget());
	SelectorWidget->SetOnSelectedOptionChangedEvent([&] (const int32 SelectedIdx) {
		if (SelectedIdx == 0)
			Selection = SelectionMode::VERTEX_EDGE;
		else if (SelectedIdx == 1)
			Selection = SelectionMode::GRAPH;
		else {
			check(false);
		}
		const auto ToolProvider = GetVrPawn()->GetToolProvider();
		const auto &HitResult = ToolProvider->GetHitResult();
		if (HitResult.bBlockingHit)
			ToolProvider->SetHitResult(HitResult);
	});
	SelectorWidget->SetButtonsEnabled(false);
	SelectorWidget->SetOptions({
		"Selection Mode: Vertex / Edge",
		"Selection Mode: Graph"
	});
	SelectorWidget->SetSelectedOptionIndex(0, true);
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
