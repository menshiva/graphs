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
void AVRPawn::PrimaryAction(const bool IsPressed) {
	// TODO
	m_RightController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	if (IsPressed) {
		UKismetSystemLibrary::PrintString(
			GetWorld(), "Right Trigger Pressed",
			true, true, FColor::Red
		);
	}
	else {
		UKismetSystemLibrary::PrintString(
			GetWorld(), "Right Trigger Released",
			true, true, FColor::Red
		);
	}
}

void AVRPawn::SecondaryAction(const bool IsPressed) {
	if (m_IsInTeleportationMode) {
		if (IsPressed) {
			if (!m_IsCameraFadeAnimationRunning) {
				m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
				CameraTeleportAnimation([&] {
					auto teleportPoint = m_LeftController->GetTeleportPoint();
					teleportPoint.Z += 111; // add player's height
					SetActorLocation(teleportPoint);
					m_LeftController->UpdateLaserPositionDirection(false);
					m_RightController->UpdateLaserPositionDirection(false);
				});
			}
		}
		else
			m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	}
}

void AVRPawn::Rotate(const float Value) {
	static bool isTurning = false;
	const float absValue = fabsf(Value);
	if (absValue >= 0.7f) {
		if (!isTurning && !m_IsCameraFadeAnimationRunning) {
			isTurning = true;
			m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
			CameraTeleportAnimation([&, Value] {
				AddActorWorldRotation({0.0f, roundf(Value) * m_RotationAngle, 0.0f});
				m_LeftController->UpdateLaserPositionDirection(false);
				m_RightController->UpdateLaserPositionDirection(false);
			});
		}
	}
	else if (isTurning) {
		isTurning = false;
	}
}

void AVRPawn::SetTeleportationMode(const bool Enabled) {
	m_IsInTeleportationMode = Enabled;
	m_LeftController->PlayHapticEffect(GetPlayerController(), m_ActionHapticScale);
	m_LeftController->SetTeleportationMode(Enabled);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::AdjustTeleportDistance(const float Delta) {
	if (m_IsInTeleportationMode && Delta != 0.0f)
		m_LeftController->AdjustTeleportLaserLength(Delta);
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
