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
	PlayerInputComponent->BindAction("KeyboardEscActionPress", IE_Pressed, this, &AVRPawn::QuitGame);
	m_LeftController->SetupInputBindings(this, PlayerInputComponent);
	m_RightController->SetupInputBindings(this, PlayerInputComponent);
}

APlayerCameraManager* AVRPawn::GetCameraManager() const {
	return GetPlayerController()->PlayerCameraManager;
}

APlayerController *AVRPawn::GetPlayerController() const {
	return Cast<APlayerController>(Controller);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::PrimaryActionPressed() {
	// TODO
	m_RightController->PlayHapticEffect(GetPlayerController());
	UKismetSystemLibrary::PrintString(
		GetWorld(), "Right Trigger Pressed",
		true, true, FColor::Red
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::PrimaryActionReleased() {
	// TODO
	m_RightController->PlayHapticEffect(GetPlayerController());
	UKismetSystemLibrary::PrintString(
		GetWorld(), "Right Trigger Released",
		true, true, FColor::Red
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::SecondaryActionPressed() {
	if (m_IsInTeleportationMode && !m_IsCameraFadeAnimationRunning) {
		m_LeftController->PlayHapticEffect(GetPlayerController());
		CameraTeleportAnimation([&] {
			SetActorLocation(m_LeftController->GetTeleportPoint());
		});
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::SecondaryActionReleased() {
	if (m_IsInTeleportationMode)
		m_LeftController->PlayHapticEffect(GetPlayerController());
}

void AVRPawn::Rotate(const float Value) {
	static bool isTurning = false;
	const float absValue = fabsf(Value);
	if (absValue >= 0.5f) {
		if (!isTurning && !m_IsCameraFadeAnimationRunning) {
			isTurning = true;
			m_LeftController->PlayHapticEffect(GetPlayerController());
			CameraTeleportAnimation([&, Value] {
				AddActorWorldRotation({0.0f, roundf(Value) * m_RotationAngle, 0.0f});
			});
		}
	}
	else if (isTurning && absValue <= 0.2f) {
		isTurning = false;
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::AdjustTeleportDistance(const float Delta) {
	if (m_IsInTeleportationMode && Delta != 0.0f)
		m_LeftController->AdjustTeleportLaserLength(Delta);
}

void AVRPawn::TurnTeleportationModeOn() {
	m_IsInTeleportationMode = true;
	m_LeftController->PlayHapticEffect(GetPlayerController());
	m_LeftController->ToggleTeleportationMode(true);
	m_RightController->ToggleLaser(false);
}

void AVRPawn::TurnTeleportationModeOff() {
	m_IsInTeleportationMode = false;
	m_LeftController->PlayHapticEffect(GetPlayerController());
	m_LeftController->ToggleTeleportationMode(false);
	m_RightController->ToggleLaser(true);
}

// ReSharper disable once CppMemberFunctionMayBeConst
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
	m_IsCameraFadeAnimationRunning = true;
	FadeCamera(1.0f);
	FTimerHandle FadeInHandle;
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&, DoAfterFadeIn] {
		DoAfterFadeIn();
		FadeCamera(0.0f);
		FTimerHandle FadeOutHandle;
		GetWorldTimerManager().SetTimer(FadeOutHandle, FTimerDelegate::CreateLambda([&] {
			m_IsCameraFadeAnimationRunning = false;
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
