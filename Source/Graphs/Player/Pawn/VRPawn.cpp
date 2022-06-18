#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPawn::AVRPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
	SetCanBeDamaged(false);

	// Create a scene component that will act as the parent for the camera and controllers
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "VRPlayerRoot");
	RootComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	RootComponent->SetRelativeScale3D(FVector::OneVector);

	// Create a camera component and attach this to the root
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	Camera->SetRelativeScale3D(FVector::OneVector);

	// Create left controller component and attach this to the root
	LeftController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	LeftController->SetupAttachment(RootComponent);
	LeftController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	LeftController->SetRelativeScale3D(FVector::OneVector);

	// Create right controller component and attach this to the root
	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupAttachment(RootComponent);
	RightController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	RightController->SetRelativeScale3D(FVector::OneVector);
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

void AVRPawn::TurnLeft() {
	if (!IsTeleporting) {
		LeftController->PlayHapticEffect(GetPlayerController());
		CameraTeleportAnimation([&] {
			AddActorWorldRotation({0.0f, -TurnAngle, 0.0f});
		});
	}
}

void AVRPawn::TurnRight() {
	if (!IsTeleporting) {
		LeftController->PlayHapticEffect(GetPlayerController());
		CameraTeleportAnimation([&] {
			AddActorWorldRotation({0.0f, TurnAngle, 0.0f});
		});
	}
}

void AVRPawn::MoveY(const float Speed) {
	if (Speed != 0.0f) {
		auto dt = Camera->GetForwardVector() * SpeedCoefficient * Speed;
		dt.Z = 0.0f;
		AddActorWorldOffset(dt);
	}
}

void AVRPawn::MoveX(const float Speed) {
	if (Speed != 0.0f) {
		auto dt = Camera->GetRightVector() * SpeedCoefficient * Speed;
		dt.Z = 0.0f;
		AddActorWorldOffset(dt);
	}
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
void AVRPawn::QuitGame() {
	UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	SetActorRelativeLocation({0.0f, 0.0f, PlayerHeight});
}

void AVRPawn::CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn) {
	IsTeleporting = true;
	FadeCamera(1.0f);
	FTimerHandle FadeInHandle;
	GetWorldTimerManager().SetTimer(FadeInHandle, FTimerDelegate::CreateLambda([&, DoAfterFadeIn] {
		DoAfterFadeIn();
		FadeCamera(0.0f);
		FTimerHandle FadeOutHandle;
		GetWorldTimerManager().SetTimer(FadeOutHandle, FTimerDelegate::CreateLambda([&] {
			IsTeleporting = false;
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
