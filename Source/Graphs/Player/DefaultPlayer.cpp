#include "DefaultPlayer.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

ADefaultPlayer::ADefaultPlayer() {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	SetCanBeDamaged(false);
	AutoPossessPlayer = EAutoReceiveInput::Type::Player0;
	bNetLoadOnClient = false;

	// The default root component for the pawn
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	RootComponent->SetRelativeScale3D(FVector::OneVector);

	// Create a scene component that will act as the parent for the camera
	const auto VrCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRCameraRoot"));
	VrCameraRoot->SetupAttachment(RootComponent);
	VrCameraRoot->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	VrCameraRoot->SetRelativeScale3D(FVector::OneVector);

	// Create a camera component and attach this to the camera root
	const auto VrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VrCamera->SetupAttachment(VrCameraRoot);
	VrCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	VrCamera->SetRelativeScale3D(FVector::OneVector);
}

void ADefaultPlayer::BeginPlay() {
	Super::BeginPlay();
	SetActorLocationAndRotation(FVector::ZeroVector, FQuat::Identity);

	// Setting up camera to work with a seated experience
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
}

// Called to bind functionality to input
void ADefaultPlayer::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
