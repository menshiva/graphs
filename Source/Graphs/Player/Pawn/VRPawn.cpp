#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Graphs/Player/Menu/MenuWidgetComponent.h"
#include "Graphs/Player/ToolController/ToolController.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPawn::AVRPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
	SetCanBeDamaged(false);

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "VRPlayerRoot");

	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	Camera->SetupAttachment(RootComponent);

	LeftController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	LeftController->SetupVrPawn(this);
	LeftController->SetupAttachment(RootComponent);

	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupVrPawn(this);
	RightController->SetupAttachment(RootComponent);

	Menu = ObjectInitializer.CreateDefaultSubobject<UMenuWidgetComponent>(this, "Menu");
	Menu->SetupAttachment(LeftController->GetMotionController());

	ToolController = ObjectInitializer.CreateDefaultSubobject<UToolController>(this, "ToolController");
	ToolController->SetupPawn(this);
}

void AVRPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	LeftController->SetupInputBindings(PlayerInputComponent);
	RightController->SetupInputBindings(PlayerInputComponent);
}

UVRControllerBase *AVRPawn::GetOtherController(const UVRControllerBase *ThisController) const {
	if (ThisController == RightController)
		return LeftController;
	return RightController;
}

void AVRPawn::ToggleCameraFadeAnimation() {
	CameraFadeAnimationEnabled = !CameraFadeAnimationEnabled;
	SaveConfig();
}

void AVRPawn::ToggleMenu() const {
	bool isMenuShown = Menu->IsVisible();
	isMenuShown = !isMenuShown;
	Menu->SetVisble(isMenuShown);
	RightController->SetUiInteractionEnabled(isMenuShown);
}

void AVRPawn::Rotate(const float Value) {
	CameraTeleportAnimation([&, Value] {
		AddActorWorldRotation({0.0f, roundf(Value) * RotationAngle, 0.0f});
		LeftController->ForceUpdateLaserTransform();
		RightController->ForceUpdateLaserTransform();
	});
}

void AVRPawn::Teleport(const FVector &Location) {
	CameraTeleportAnimation([&] {
		SetActorLocation(Location);
		LeftController->ForceUpdateLaserTransform();
		RightController->ForceUpdateLaserTransform();
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

bool AVRPawn::OnLeftTriggerAction(const bool IsPressed) {
	return ToolController->OnLeftTriggerAction(IsPressed);
}

bool AVRPawn::OnRightTriggerAction(const bool IsPressed) {
	return ToolController->OnRightTriggerAction(IsPressed);
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
