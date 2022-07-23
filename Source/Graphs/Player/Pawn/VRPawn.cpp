#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Graphs/Provider/Commands/NodeCommands.h"
#include "Graphs/UI/MenuWidgetComponent.h"
#include "Kismet/GameplayStatics.h"
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
	LeftController->SetupPawn(this);
	LeftController->SetupAttachment(RootComponent);

	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupPawn(this);
	RightController->SetupAttachment(RootComponent);

	Menu = ObjectInitializer.CreateDefaultSubobject<UMenuWidgetComponent>(this, "Menu");
	Menu->SetVisibility(false);
	Menu->SetupAttachment(LeftController->MotionController);
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

bool AVRPawn::OnLeftMenuPressed() {
	static bool isMenuShown = false;
	isMenuShown = !isMenuShown;
	Menu->SetVisble(isMenuShown);
	RightController->SetUiInteractionEnabled(isMenuShown);
	return true;
}

bool AVRPawn::OnLeftTriggerAction(const bool IsPressed) {
	if (LeftController->GetHitResult().GetActor() != nullptr) {
		LeftController->SetState(IsPressed ? ControllerState::TOOL : ControllerState::NONE);
		// TODO: pass to active tool
		return true;
	}
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

bool AVRPawn::OnRightTriggerAction(const bool IsPressed) {
	if (RightController->GetHitResult().GetActor() != nullptr) {
		RightController->SetState(IsPressed ? ControllerState::TOOL : ControllerState::NONE);
		// TODO: pass to active tool
		return true;
	}
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

void AVRPawn::OnEntityHitChanged(const UVRControllerBase *ControllerHit, const AEntity *Entity, const bool IsHit) const {
	// TODO: pass to tool controller and decide if we should hit based on active tool
	if (const auto PrimitiveComponent = Cast<UPrimitiveComponent>(Entity->GetStaticMeshComponent())) {
		if (IsHit)
			ControllerHit->PlayActionHapticEffect();
		UVRControllerBase *OtherController;
		if (ControllerHit == RightController)
			OtherController = LeftController;
		else
			OtherController = RightController;
		if (OtherController->GetHitResult().GetActor() != Entity)
			PrimitiveComponent->SetRenderCustomDepth(IsHit);
	}
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	CachedProvider = Cast<AGraphProvider>(UGameplayStatics::GetActorOfClass(GetWorld(), AGraphProvider::StaticClass()));

	// TODO: only for test
	{
		const FVector Positions[] = {
			{437.109619f, 225.096985f, 50.0f},
			{748.974915f, 345.263428f, 260.0f},
			{504.859009f, -437.556763f, 460.0f},
			{969.929321f, -452.031494f, 260.0f},
			{1587.086426f, 611.200684f, 440.0f},
			{1903.230957f, 502.790161f, 650.0f},
			{1213.039551f, 60.030151f, 850.0f},
			{1560.0f, -250.0f, 650.0f},
		};
		for (const auto &Pos : Positions)
			CachedProvider->PushCommand<NodeCommands::Create>(nullptr, Pos);
		CachedProvider->MarkDirty();
	}
}

void AVRPawn::FadeCamera(const float ToValue) const {
	GetPlayerController()->PlayerCameraManager->StartCameraFade(
		1.0 - ToValue, ToValue,
		ScreenFadeDuration, FColor::Black,
		false, static_cast<bool>(ToValue)
	);
}
