#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPawn::AVRPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
	SetCanBeDamaged(false);

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "VRPlayerRoot");

	m_Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	m_Camera->SetupAttachment(RootComponent);

	m_LeftController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	m_LeftController->SetupAttachment(RootComponent);

	m_RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	m_RightController->SetupAttachment(RootComponent);
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	PlayerInputComponent->BindAction("Menu", IE_Pressed, this, &AVRPawn::ToggleMenu);
	PlayerInputComponent->BindAction("KeyboardEsc", IE_Pressed, this, &AVRPawn::QuitGame);
	m_LeftController->SetupInputBindings(this, PlayerInputComponent);
	m_RightController->SetupInputBindings(this, PlayerInputComponent);
}

APlayerCameraManager* AVRPawn::GetCameraManager() const {
	return GetPlayerController()->PlayerCameraManager;
}

APlayerController *AVRPawn::GetPlayerController() const {
	return Cast<APlayerController>(Controller);
}

bool AVRPawn::OnRightTriggerPressed() {
	// TODO: pass event to active tool
	if (m_RightController->GetState() == ControllerState::UI) {
		m_RightController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
		m_RightController->UiLeftMouseButtonPress();
		return true;
	}
	return InputInterface::OnRightTriggerPressed();
}

bool AVRPawn::OnRightTriggerReleased() {
	// TODO: pass event to active tool
	m_RightController->UiLeftMouseButtonRelease();
	return InputInterface::OnRightTriggerReleased();
}

bool AVRPawn::OnLeftTriggerPressed() {
	// TODO: pass event to active tool
	if (m_LeftController->GetState() == ControllerState::TELEPORTATION) {
		if (!m_IsCameraFadeAnimationRunning) {
			m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
			CameraTeleportAnimation([&] {
				auto teleportPoint = m_LeftController->GetLaserEndPosition();
				teleportPoint.Z += 111; // add player's height
				SetActorLocation(teleportPoint);
				m_LeftController->UpdateLaser(false);
				m_RightController->UpdateLaser(false);
			});
			return true;
		}
	}
	return InputInterface::OnLeftTriggerPressed();
}

bool AVRPawn::OnLeftTriggerReleased() {
	// TODO: pass event to active tool
	return InputInterface::OnLeftTriggerReleased();
}

bool AVRPawn::OnLeftGripPressed() {
	// TODO: pass event to active tool
	m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	m_LeftController->SetState(ControllerState::TELEPORTATION);
	return InputInterface::OnLeftGripPressed();
}

bool AVRPawn::OnLeftGripReleased() {
	// TODO: pass event to active tool
	m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	m_LeftController->SetState(ControllerState::NONE);
	return InputInterface::OnLeftGripReleased();
}

bool AVRPawn::OnLeftThumbstickY(const float Value) {
	// TODO: pass OnLeftThumbstickY event to active tool
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked) {
			isClicked = true;
			if (Value > 0.0f) {
				if (OnLeftThumbstickUp())
					return true;
			}
			else {
				if (OnLeftThumbstickDown())
					return true;
			}
			return true;
		}
	}
	else if (isClicked) {
		isClicked = false;
	}
	if (m_LeftController->GetState() == ControllerState::TELEPORTATION) {
		m_LeftController->AdjustTeleportLaserLength(Value);
		return true;
	}
	return InputInterface::OnLeftThumbstickY(Value);
}

bool AVRPawn::OnLeftThumbstickX(const float Value) {
	// TODO: pass OnLeftThumbstickX event to active tool
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked && !m_IsCameraFadeAnimationRunning) {
			isClicked = true;
			if (Value > 0.0f) {
				if (OnLeftThumbstickRight())
					return true;
			}
			else {
				if (OnLeftThumbstickLeft())
					return true;
			}
			m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
			CameraTeleportAnimation([&, Value] {
				AddActorWorldRotation({0.0f, roundf(Value) * m_RotationAngle, 0.0f});
				m_LeftController->UpdateLaser(false);
				m_RightController->UpdateLaser(false);
			});
			return true;
		}
	}
	else if (isClicked) {
		isClicked = false;
		return true;
	}
	return InputInterface::OnLeftThumbstickX(Value);
}

bool AVRPawn::OnLeftThumbstickLeft() {
	// TODO: pass event to active tool
	return InputInterface::OnLeftThumbstickLeft();
}

bool AVRPawn::OnLeftThumbstickRight() {
	// TODO: pass event to active tool
	return InputInterface::OnLeftThumbstickRight();
}

bool AVRPawn::OnLeftThumbstickUp() {
	// TODO: pass event to active tool
	return InputInterface::OnLeftThumbstickUp();
}

bool AVRPawn::OnLeftThumbstickDown() {
	// TODO: pass event to active tool
	return InputInterface::OnLeftThumbstickDown();
}

// ReSharper disable once CppUE4BlueprintCallableFunctionMayBeConst
void AVRPawn::QuitGame() {
	UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
}

void AVRPawn::CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn) {
	FTimerHandle FadeInHandle;
	m_IsCameraFadeAnimationRunning = true;
	FadeCamera(1.0f);
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&, DoAfterFadeIn] {
		DoAfterFadeIn();
		FTimerHandle WaitHandle;
		GetWorldTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&] {
			FadeCamera(0.0f);
			FTimerHandle FadeOutHandle;
			GetWorldTimerManager().SetTimer(FadeOutHandle, FTimerDelegate::CreateLambda([&] {
				m_IsCameraFadeAnimationRunning = false;
			}), m_ScreenFadeDuration, false);
		}), m_ScreenFadeDuration, false);
	}), m_ScreenFadeDuration, false);
}

void AVRPawn::FadeCamera(const float Value) const {
	GetCameraManager()->StartCameraFade(
		1.0 - Value, Value,
		m_ScreenFadeDuration, FColor::Black,
		false, static_cast<bool>(Value)
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::ToggleMenu() {
	m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	static bool isMenuShown = false;
	isMenuShown = !isMenuShown;
	m_RightController->SetUiInteractionEnabled(isMenuShown);
	if (isMenuShown)
		m_LeftController->SpawnMainMenu(this);
	else
		m_LeftController->DestroyMainMenu();
}
