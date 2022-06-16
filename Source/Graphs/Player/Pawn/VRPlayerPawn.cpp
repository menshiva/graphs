#include "VRPlayerPawn.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

AVRPlayerPawn::AVRPlayerPawn(const FObjectInitializer &ObjectInitializer) : APawn(ObjectInitializer) {
	SetActorTickEnabled(false);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	bAllowTickBeforeBeginPlay = false;
	SetCanBeDamaged(false);
	AutoPossessPlayer = EAutoReceiveInput::Type::Player0;
	bNetLoadOnClient = false;

	// Create a scene component that will act as the parent for the camera and controllers
	Root = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "VRPlayerRoot");
	Root->SetupAttachment(RootComponent);
	Root->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	Root->SetRelativeScale3D(FVector::OneVector);

	// Create a camera component and attach this to the root
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, "VRCamera");
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	Camera->SetRelativeScale3D(FVector::OneVector);

	// Create left controller component and attach this to the root
	LeftController = ObjectInitializer.CreateDefaultSubobject<UVRControllerLeft>(this, "VRLeftController");
	LeftController->SetupAttachment(Root);
	LeftController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	LeftController->SetRelativeScale3D(FVector::OneVector);

	// Create right controller component and attach this to the root
	RightController = ObjectInitializer.CreateDefaultSubobject<UVRControllerRight>(this, "VRRightController");
	RightController->SetupAttachment(Root);
	RightController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	RightController->SetRelativeScale3D(FVector::OneVector);
}

void AVRPlayerPawn::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("KeyboardEscActionPress", IE_Pressed, this, &AVRPlayerPawn::QuitGame);
	LeftController->SetupInputBindings(this, PlayerInputComponent);
	RightController->SetupInputBindings(this, PlayerInputComponent);
}

APlayerController *AVRPlayerPawn::GetPlayerController() const {
	return Cast<APlayerController>(Controller);
}

void AVRPlayerPawn::TurnLeft() {
	AddActorWorldRotation({0.0f, -45.0f, 0.0f});
}

void AVRPlayerPawn::TurnRight() {
	AddActorWorldRotation({0.0f, 45.0f, 0.0f});
}

void AVRPlayerPawn::MoveY(const float Speed) {
	if (Speed != 0.0f) {
		auto dt = Camera->GetForwardVector() * SpeedCoefficient * Speed;
		dt.Z = 0.0f;
		AddActorWorldOffset(dt);
	}
}

void AVRPlayerPawn::MoveX(const float Speed) {
	if (Speed != 0.0f) {
		auto dt = Camera->GetRightVector() * SpeedCoefficient * Speed;
		dt.Z = 0.0f;
		AddActorWorldOffset(dt);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AVRPlayerPawn::QuitGame() {
	UKismetSystemLibrary::QuitGame(GetWorld(), GetPlayerController(), EQuitPreference::Type::Quit, false);
}

void AVRPlayerPawn::BeginPlay() {
	Super::BeginPlay();
	SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()) {
		// Setting up camera to work with a seated experience
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	}
}
