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

	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	Camera->SetupAttachment(RootComponent);

	LeftController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	LeftController->SetupAttachment(RootComponent);

	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupAttachment(RootComponent);
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	PlayerInputComponent->BindAction("KeyboardEscActionPress", IE_Pressed, this, &AVRPawn::QuitGame);
	LeftController->SetupInputBindings(this, PlayerInputComponent);
	RightController->SetupInputBindings(this, PlayerInputComponent);
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
	RightController->PlayHapticEffect(GetPlayerController());
	UKismetSystemLibrary::PrintString(
		GetWorld(), "Right Trigger Pressed",
		true, true, FColor::Red
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::PrimaryActionReleased() {
	// TODO
	RightController->PlayHapticEffect(GetPlayerController());
	UKismetSystemLibrary::PrintString(
		GetWorld(), "Right Trigger Released",
		true, true, FColor::Red
	);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::SecondaryActionPressed() {
	if (IsInTeleportationMode && !IsCameraFadeAnimationRunning) {
		LeftController->PlayHapticEffect(GetPlayerController());
		CameraTeleportAnimation([&] {
			SetActorLocation(LeftController->GetTeleportPoint());
		});
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::SecondaryActionReleased() {
	if (IsInTeleportationMode)
		LeftController->PlayHapticEffect(GetPlayerController());
}

void AVRPawn::Rotate(const float Value) {
	static bool isTurning = false;
	const float absValue = fabsf(Value);
	if (absValue >= 0.5f) {
		if (!isTurning && !IsCameraFadeAnimationRunning) {
			isTurning = true;
			LeftController->PlayHapticEffect(GetPlayerController());
			CameraTeleportAnimation([&, Value] {
				AddActorWorldRotation({0.0f, roundf(Value) * RotationAngle, 0.0f});
			});
		}
	}
	else if (isTurning && absValue <= 0.2f) {
		isTurning = false;
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::AdjustTeleportDistance(const float Delta) {
	if (IsInTeleportationMode && Delta != 0.0f)
		LeftController->SetTeleportLaserDistance(Delta);
}

void AVRPawn::TurnTeleportationModeOn() {
	IsInTeleportationMode = true;
	LeftController->PlayHapticEffect(GetPlayerController());
	LeftController->ToggleMeshInteractionLaser(false);
	RightController->ToggleMeshInteractionLaser(false);
	LeftController->ToggleTeleportationMode(true);
}

void AVRPawn::TurnTeleportationModeOff() {
	IsInTeleportationMode = false;
	LeftController->PlayHapticEffect(GetPlayerController());
	LeftController->ToggleMeshInteractionLaser(true);
	RightController->ToggleMeshInteractionLaser(true);
	LeftController->ToggleTeleportationMode(false);
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
	IsCameraFadeAnimationRunning = true;
	FadeCamera(1.0f);
	FTimerHandle FadeInHandle;
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&, DoAfterFadeIn] {
		DoAfterFadeIn();
		FadeCamera(0.0f);
		FTimerHandle FadeOutHandle;
		GetWorldTimerManager().SetTimer(FadeOutHandle, FTimerDelegate::CreateLambda([&] {
			IsCameraFadeAnimationRunning = false;
		}), ScreenFadeDuration, false);
	}), ScreenFadeDuration, false);
}

void AVRPawn::FadeCamera(const float Value) const {
	GetCameraManager()->StartCameraFade(
		1.0 - Value, Value,
		ScreenFadeDuration, FColor::Black,
		false, static_cast<bool>(Value)
	);
}
