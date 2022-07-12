#include "VRControllerLeft.h"
#include "../Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : USceneComponent(ObjectInitializer), UVRControllerBase(ObjectInitializer, this, "Left") {
	PrimaryComponentTick.bCanEverTick = true;

	const ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Game/Graphs/Meshes/Capsule"));
	m_TeleportPreviewMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "TeleportPreviewMesh");
	m_TeleportPreviewMesh->SetStaticMesh(TeleportPreviewMeshAsset.Object);
	m_TeleportPreviewMesh->SetVisibility(false);
	m_TeleportPreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	m_TeleportPreviewMesh->SetCastShadow(false);
	m_TeleportPreviewMesh->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/Pawn/TeleportPreviewMaterial")
	);
	const auto TeleportPreviewMaterialInst = m_TeleportPreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		TeleportPreviewMaterialAsset.Object
	);
	TeleportPreviewMaterialInst->SetVectorParameterValue("Color", m_TeleportLaserColor);
	m_TeleportPreviewMesh->SetMaterial(0, TeleportPreviewMaterialInst);
}

void UVRControllerLeft::SetupInputBindings(APawn *Pawn, UInputComponent *Pic) const {
	const auto vrPawn = Cast<AVRPawn>(Pawn);

	BindAction(Pic, "LeftTrigger", IE_Pressed, [vrPawn] {
		vrPawn->OnLeftTriggerPressed();
	});
	BindAction(Pic, "LeftTrigger", IE_Released, [vrPawn] {
		vrPawn->OnLeftTriggerReleased();
	});

	BindAction(Pic, "LeftGrip", IE_Pressed, [vrPawn] {
		vrPawn->OnLeftGripPressed();
	});
	BindAction(Pic, "LeftGrip", IE_Released, [vrPawn] {
		vrPawn->OnLeftGripReleased();
	});

	BindAxis(Pic, "LeftThumbstickAxisY", [vrPawn] (const float Value) {
		vrPawn->OnLeftThumbstickY(Value);
	});
	BindAxis(Pic, "LeftThumbstickAxisX", [vrPawn] (const float Value) {
		vrPawn->OnLeftThumbstickX(Value);
	});
}

void UVRControllerLeft::PlayHapticEffect(APlayerController *PlayerController, const float Scale) {
	PlayerController->PlayHapticEffect(GetHapticEffectController(), EControllerHand::Left, Scale);
}

void UVRControllerLeft::SetState(const ControllerState NewState) {
	UVRControllerBase::SetState(NewState);
	if (NewState != ControllerState::TELEPORTATION) {
		SetLaserColor(m_MeshInteractionLaserColor);
		SetLaserLength(m_MeshInteractionLaserMaxDistance);
		m_TeleportPreviewMesh->SetVisibility(false);
	}
	else {
		SetLaserColor(m_TeleportLaserColor);
		SetLaserLength(m_TeleportLaserCurrentDistance);
		m_TeleportPreviewMesh->SetVisibility(true);
	}
}

void UVRControllerLeft::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaser();
	if (GetState() == ControllerState::TELEPORTATION)
		m_TeleportPreviewMesh->SetWorldLocation(GetLaserEndPosition());
}

void UVRControllerLeft::AdjustTeleportLaserLength(const float Delta) {
	m_TeleportLaserCurrentDistance = FMath::Clamp(
		m_TeleportLaserCurrentDistance + Delta * m_TeleportLaserLengthDeltaSpeed,
		m_TeleportLaserMinDistance,
		m_TeleportLaserMaxDistance
	);
	SetLaserLength(m_TeleportLaserCurrentDistance);
}
