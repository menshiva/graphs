#include "VRPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPawn::AVRPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;
	SetCanBeDamaged(false);

	ControllerActionHapticEffect = ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base>(TEXT(
		"/Game/Haptics/ControllerActionHapticEffect"
	)).Object;

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
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("KeyboardEscActionPress", IE_Pressed, this, &AVRPawn::QuitGame);
	LeftController->SetupInputBindings(this, PlayerInputComponent);
	RightController->SetupInputBindings(this, PlayerInputComponent);
}

APlayerController *AVRPawn::GetPlayerController() const {
	return Cast<APlayerController>(Controller);
}

UVRControllerLeft *AVRPawn::GetLeftController() const {
	return LeftController;
}

UVRControllerRight *AVRPawn::GetRightController() const {
	return RightController;
}

void AVRPawn::TurnLeft() {
	AddActorWorldRotation({0.0f, -45.0f, 0.0f});
	GetPlayerController()->PlayHapticEffect(ControllerActionHapticEffect, EControllerHand::Left);
}

void AVRPawn::TurnRight() {
	AddActorWorldRotation({0.0f, 45.0f, 0.0f});
	GetPlayerController()->PlayHapticEffect(ControllerActionHapticEffect, EControllerHand::Left);
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
void AVRPawn::PrimaryAction() {
	// TODO
	UKismetSystemLibrary::PrintString(
		GetWorld(), "Right Trigger Pressed",
		true, true, FColor::Red
	);
	GetPlayerController()->PlayHapticEffect(ControllerActionHapticEffect, EControllerHand::Right);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPawn::QuitGame() {
	UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
}

void AVRPawn::BeginPlay() {
	Super::BeginPlay();
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	SetActorRelativeLocation({0.0f, 0.0f, 111.0f});
}
