#include "DefaultPlayer.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ADefaultPlayer::ADefaultPlayer() {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetCanBeDamaged(false);
	AutoPossessPlayer = EAutoReceiveInput::Type::Player0;
	bNetLoadOnClient = false;

	const auto VrCameraRoot = CreatePawn();
	CreateHandController(VrCameraRoot, "VRMotionControllerLeft", FXRMotionControllerBase::LeftHandSourceId);
	CreateHandController(VrCameraRoot, "VRMotionControllerRight", FXRMotionControllerBase::RightHandSourceId);
}

void ADefaultPlayer::BeginPlay() {
	Super::BeginPlay();
	SetActorLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()) {
		// Setting up camera to work with a seated experience
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	}
}

// Called to bind functionality to input
void ADefaultPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// ReSharper disable CppUENonExistentInputAction
	InputComponent->BindAction(
		"LeftThumbstickActionLeft", IE_Pressed,
		this, &ADefaultPlayer::OnLeftControllerThumbstickLeft
	);
	InputComponent->BindAction(
		"LeftThumbstickActionRight", IE_Pressed,
		this, &ADefaultPlayer::OnLeftControllerThumbstickRight
	);
	InputComponent->BindAction(
		"RightTriggerActionClick", IE_Pressed,
		this, &ADefaultPlayer::OnRightControllerTriggerPressed
	);
	InputComponent->BindAction(
		"RightTriggerActionClick", IE_Released,
		this, &ADefaultPlayer::OnRightControllerTriggerReleased
	);
	InputComponent->BindAction(
		"KeyboardEscActionPress", IE_Pressed,
		this, &ADefaultPlayer::OnKeyboardEscPressed
	);
}

USceneComponent* ADefaultPlayer::CreatePawn() {
	// The default root component for the pawn
	RootComponent = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	RootComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	RootComponent->SetRelativeScale3D(FVector::OneVector);

	// Create a scene component that will act as the parent for the camera
	const auto VrCameraRoot = CreateDefaultSubobject<USceneComponent>("VRCameraRoot");
	VrCameraRoot->SetupAttachment(RootComponent);
	VrCameraRoot->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	VrCameraRoot->SetRelativeScale3D(FVector::OneVector);

	// Create a camera component and attach this to the camera root
	const auto VrCamera = CreateDefaultSubobject<UCameraComponent>("VRCamera");
	VrCamera->SetupAttachment(VrCameraRoot);
	VrCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	VrCamera->SetRelativeScale3D(FVector::OneVector);

	return VrCameraRoot;
}

void ADefaultPlayer::CreateHandController(
	USceneComponent *VrCameraRoot,
	const FName &ObjectName,
	const FName &HandType
) {
	const auto MotionController = CreateDefaultSubobject<UMotionControllerComponent>(ObjectName);
	MotionController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	MotionController->SetRelativeScale3D(FVector::OneVector);
	MotionController->MotionSource = HandType;
	MotionController->SetupAttachment(VrCameraRoot);
	MotionController->SetShowDeviceModel(true);
}

void ADefaultPlayer::OnLeftControllerThumbstickLeft() {
	UE_LOG(LogTemp, Log, TEXT("Left Controller Thumbstick Left Event"));
	AddActorWorldRotation({0.0f, -45.0f, 0.0f});
}

void ADefaultPlayer::OnLeftControllerThumbstickRight() {
	UE_LOG(LogTemp, Log, TEXT("Left Controller Thumbstick Right Event"));
	AddActorWorldRotation({0.0f, 45.0f, 0.0f});
}

void ADefaultPlayer::OnRightControllerTriggerPressed() {
	UE_LOG(LogTemp, Log, TEXT("Right Controller Trigger Press Event"));
	// TODO
}

void ADefaultPlayer::OnRightControllerTriggerReleased() {
	UE_LOG(LogTemp, Log, TEXT("Right Controller Trigger Release Event"));
	// TODO
}

void ADefaultPlayer::OnKeyboardEscPressed() {
	UE_LOG(LogTemp, Log, TEXT("Keyboard Escape Press Event"));
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Type::Quit, false);
}
