#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPawn::AVRPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
	SetCanBeDamaged(false);

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "VRPlayerRoot");

	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	Camera->SetupAttachment(RootComponent);

	LeftVrController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	LeftVrController->SetupVrPawn(this);
	LeftVrController->SetupAttachment(RootComponent);

	RightVrController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightVrController->SetupVrPawn(this);
	RightVrController->SetupAttachment(RootComponent);

	Menu = ObjectInitializer.CreateDefaultSubobject<UMenuWidgetComponent>(this, "Menu");
	Menu->SetupAttachment(LeftVrController->GetMotionController());

	ToolProvider = ObjectInitializer.CreateDefaultSubobject<UToolProvider>(this, "ToolProvider");
	ToolProvider->SetupPawn(this);
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	LeftVrController->SetupInputBindings(PlayerInputComponent);
	RightVrController->SetupInputBindings(PlayerInputComponent);
}

void AVRPawn::ToggleCameraFadeAnimation() {
	CameraFadeAnimationEnabled = !CameraFadeAnimationEnabled;
	SaveConfig();
}

void AVRPawn::ToggleMenu() const {
	bool isMenuShown = Menu->IsVisible();
	isMenuShown = !isMenuShown;
	RightVrController->SetUiInteractionEnabled(isMenuShown);
	Menu->SetVisble(isMenuShown);
}

void AVRPawn::Rotate(const float Value) {
	CameraTeleportAnimation([&, Value] {
		AddActorWorldRotation({0.0f, roundf(Value) * RotationAngle, 0.0f});
		LeftVrController->ForceUpdateLaserTransform();
		RightVrController->ForceUpdateLaserTransform();
	});
}

void AVRPawn::Teleport(const FVector &Location) {
	CameraTeleportAnimation([&] {
		SetActorLocation(Location);
		LeftVrController->ForceUpdateLaserTransform();
		RightVrController->ForceUpdateLaserTransform();
	});
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::QuitGame() {
	FTimerHandle FadeInHandle;
	FadeCamera(1.0f);
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&] {
		UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
	}), ScreenFadeDuration, false);
}

bool AVRPawn::OnRightTriggerAction(const bool IsPressed) {
	return ToolProvider->OnRightTriggerAction(IsPressed);
}

bool AVRPawn::OnRightThumbstickY(const float Value) {
	return ToolProvider->OnRightThumbstickY(Value);
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
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

void AVRPawn::FadeCamera(const float ToValue) const {
	GetPlayerController()->PlayerCameraManager->StartCameraFade(
		1.0 - ToValue, ToValue,
		ScreenFadeDuration, FColor::Black,
		false, static_cast<bool>(ToValue)
	);
}
