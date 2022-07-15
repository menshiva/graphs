#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Graphs/UI/MenuWidgetComponent.h"
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
	LeftController->SetupPawn(this);
	LeftController->SetupAttachment(RootComponent);

	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupPawn(this);
	RightController->SetupAttachment(RootComponent);

	Menu = ObjectInitializer.CreateDefaultSubobject<UMenuWidgetComponent>(this, "Menu");
	Menu->SetVisibility(false);
	Menu->SetupAttachment(LeftController->MotionController.Get());
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	PlayerInputComponent->BindAction("KeyboardEsc", IE_Pressed, this, &AVRPawn::QuitGame);
	LeftController->SetupInputBindings(PlayerInputComponent);
	RightController->SetupInputBindings(PlayerInputComponent);
}

APlayerController *AVRPawn::GetPlayerController() const {
	return Cast<APlayerController>(Controller);
}

void AVRPawn::CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn) {
	if (CameraFadeAnimationEnabled) {
		if (!IsCameraFadeAnimationRunning) {
			FTimerHandle FadeInHandle;
			IsCameraFadeAnimationRunning = true;
			FadeCamera(1.0f);
			GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&, DoAfterFadeIn] {
				DoAfterFadeIn();
				FTimerHandle WaitHandle;
				GetWorldTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&] {
					FadeCamera(0.0f);
					FTimerHandle FadeOutHandle;
					GetWorldTimerManager().SetTimer(FadeOutHandle, FTimerDelegate::CreateLambda([&] {
						IsCameraFadeAnimationRunning = false;
					}), ScreenFadeDuration, false);
				}), ScreenFadeDuration, false);
			}), ScreenFadeDuration, false);
		}
	}
	else DoAfterFadeIn();
}

void AVRPawn::ToggleCameraFadeAnimation() {
	CameraFadeAnimationEnabled = !CameraFadeAnimationEnabled;
	SaveConfig();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::QuitGame() {
	FTimerHandle FadeInHandle;
	FadeCamera(1.0f);
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&] {
		UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
	}), ScreenFadeDuration, false);
}

void AVRPawn::OnLeftStateChanged(const ControllerState NewState) {
	LeftControllerInputInterface::OnLeftStateChanged(NewState);
	// TODO: pass to active tool
}

bool AVRPawn::OnLeftMenuPressed() {
	static bool isMenuShown = false;
	isMenuShown = !isMenuShown;
	Menu->SetVisble(isMenuShown);
	RightController->SetUiInteractionEnabled(isMenuShown);
	return true;
}

bool AVRPawn::OnLeftTriggerAction(const bool IsPressed) {
	if (IsPressed && LeftController->GetState() == ControllerState::TELEPORTATION) {
		CameraTeleportAnimation([&] {
			auto teleportPoint = LeftController->GetTeleportLocation();
			teleportPoint.Z += Height;
			SetActorLocation(teleportPoint);
			LeftController->UpdateLaser(false);
			RightController->UpdateLaser(false);
		});
		return true;
	}
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftTriggerAction(IsPressed);
}

bool AVRPawn::OnLeftGripAction(const bool IsPressed) {
	LeftController->SetState(IsPressed ? ControllerState::TELEPORTATION : ControllerState::NONE);
	// TODO: pass to active tool
	return true;
}

bool AVRPawn::OnLeftThumbstickYAction(const float Value) {
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftThumbstickYAction(Value);
}

bool AVRPawn::OnLeftThumbstickXAction(const float Value) {
	CameraTeleportAnimation([&, Value] {
		AddActorWorldRotation({0.0f, roundf(Value) * RotationAngle, 0.0f});
		LeftController->UpdateLaser(false);
		RightController->UpdateLaser(false);
	});
	// TODO: pass to active tool
	return true;
}

bool AVRPawn::OnLeftThumbstickYAxis(const float Value) {
	if (LeftController->GetState() == ControllerState::TELEPORTATION) {
		LeftController->AdjustTeleportLaserLength(Value);
		return true;
	}
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftThumbstickYAxis(Value);
}

bool AVRPawn::OnLeftThumbstickXAxis(const float Value) {
	// TODO: pass to active tool
	return LeftControllerInputInterface::OnLeftThumbstickXAxis(Value);
}

void AVRPawn::OnRightStateChanged(const ControllerState NewState) {
	RightControllerInputInterface::OnRightStateChanged(NewState);
	// TODO: pass to active tool
}

bool AVRPawn::OnRightTriggerAction(const bool IsPressed) {
	if (RightController->GetState() == ControllerState::UI) {
		RightController->SetUiInteractorPointerKeyPressed(IsPressed, EKeys::LeftMouseButton);
		return true;
	}
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightTriggerAction(IsPressed);
}

bool AVRPawn::OnRightThumbstickYAction(const float Value) {
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightThumbstickYAction(Value);
}

bool AVRPawn::OnRightThumbstickXAction(const float Value) {
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightThumbstickXAction(Value);
}

bool AVRPawn::OnRightThumbstickYAxis(const float Value) {
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightThumbstickYAxis(Value);
}

bool AVRPawn::OnRightThumbstickXAxis(const float Value) {
	// TODO: pass to active tool
	return RightControllerInputInterface::OnRightThumbstickXAxis(Value);
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
}

void AVRPawn::FadeCamera(const float ToValue) const {
	GetPlayerController()->PlayerCameraManager->StartCameraFade(
		1.0 - ToValue, ToValue,
		ScreenFadeDuration, FColor::Black,
		false, static_cast<bool>(ToValue)
	);
}
