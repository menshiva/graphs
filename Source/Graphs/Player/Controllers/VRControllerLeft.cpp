#include "VRControllerLeft.h"
#include "../Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Left) {
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/VFX/Laser"));
	TeleportLaser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(
		this, "TeleportLaser"
	);
	TeleportLaser->SetupAttachment(this);
	TeleportLaser->SetAsset(LaserAsset.Object);
	TeleportLaser->SetVisibility(TeleportLaserVisibility);
	TeleportLaser->SetColorParameter("User.CustomColor", TeleportLaserColor);

	// TODO init teleport preview capsule

	/*const auto mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/Models/Gizmo/Sphere")).Object;
	m_pSphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tool Draw Curve Sphere Mesh"));
	m_pSphereMesh->SetVisibility(false);
	m_pSphereMesh->SetStaticMesh(mesh);
	m_pSphereMesh->SetCollisionProfileName(TEXT("NoCollision"));
	m_pSphereMesh->SetCastShadow(false);
	m_pSphereMesh->SetCastInsetShadow(false);*/

	/*static ConstructorHelpers::FClassFinder<UUserWidget> WidgetMenuAsset(TEXT("/Game/UI/WidgetMenu"));
	WidgetMenu = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(
		this, "Menu"
	);
	WidgetMenu->SetWidgetClass(WidgetMenuAsset.Class);
	WidgetMenu->SetDrawAtDesiredSize(true);
	WidgetMenu->SetupAttachment(GetMotionControllerAim());
	WidgetMenu->SetRelativeRotation({0.0f, 180.0f, 0.0f});
	WidgetMenu->SetRelativeScale3D(FVector(0.08f));*/
}

void UVRControllerLeft::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {
	const auto vrPawn = Cast<AVRPawn>(Pawn);
	PlayerInputComponent->BindAxis("LeftThumbstickAxisY", vrPawn, &AVRPawn::AdjustTeleportDistance);
	PlayerInputComponent->BindAxis("LeftThumbstickAxisX", vrPawn, &AVRPawn::Rotate);
	PlayerInputComponent->BindAction("LeftTriggerActionPress", IE_Pressed, vrPawn, &AVRPawn::SecondaryActionPressed);
	PlayerInputComponent->BindAction("LeftTriggerActionPress", IE_Released, vrPawn, &AVRPawn::SecondaryActionReleased);
	PlayerInputComponent->BindAction("LeftGripActionPress", IE_Pressed, vrPawn, &AVRPawn::TurnTeleportationModeOn);
	PlayerInputComponent->BindAction("LeftGripActionPress", IE_Released, vrPawn, &AVRPawn::TurnTeleportationModeOff);
}

void UVRControllerLeft::ToggleTeleportationMode(const bool Enable) {
	TeleportLaserVisibility = Enable;
	TeleportLaser->SetVisibility(Enable);
	// TODO: Set visibility capsule
}

void UVRControllerLeft::SetTeleportLaserDistance(const float Delta) {
	TeleportLaserCurrentDistance = FMath::Clamp(
		TeleportLaserCurrentDistance + Delta * TeleportLaserDistanceSpeed,
		TeleportLaserMinDistance,
		TeleportLaserMaxDistance
	);
}

FVector UVRControllerLeft::GetTeleportPoint() const {
	return GetMotionControllerAimEndPos(TeleportLaserCurrentDistance);
}

void UVRControllerLeft::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (TeleportLaserVisibility) {
		UpdateLaserPositions(
			TeleportLaser,
			GetMotionControllerAimStartPos(),
			GetMotionControllerAimEndPos(TeleportLaserCurrentDistance)
		);
		// TODO: Move capsule
		// TODO: add laser and capsule lerp
	}
}
