#include "VRControllerLeft.h"
#include "../Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Left) {
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Game/Meshes/Capsule"));
	m_TeleportPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>("TeleportPreviewMesh");
	m_TeleportPreviewMesh->SetupAttachment(this);
	m_TeleportPreviewMesh->SetStaticMesh(TeleportPreviewMeshAsset.Object);
	m_TeleportPreviewMesh->SetVisibility(false);
	m_TeleportPreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	m_TeleportPreviewMesh->SetCastShadow(false);
	m_TeleportPreviewMesh->SetCastInsetShadow(false);

	static ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Materials/TeleportPreviewMaterial")
	);
	const auto TeleportPreviewMaterialInst = m_TeleportPreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		TeleportPreviewMaterialAsset.Object
	);
	TeleportPreviewMaterialInst->SetVectorParameterValue("Color", m_TeleportLaserColor);
	m_TeleportPreviewMesh->SetMaterial(0, TeleportPreviewMaterialInst);
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

void UVRControllerLeft::SetTeleportationMode(const bool Enable) const {
	m_Laser->SetColor(Enable ? m_TeleportLaserColor : m_MeshInteractionLaserColor);
	m_Laser->SetLength(Enable ? m_TeleportLaserCurrentDistance : m_MeshInteractionLaserMaxDistance);
	m_TeleportPreviewMesh->SetVisibility(Enable);
}

void UVRControllerLeft::AdjustTeleportLaserLength(const float Delta) {
	m_TeleportLaserCurrentDistance = FMath::Clamp(
		m_TeleportLaserCurrentDistance + Delta * m_TeleportLaserLengthDeltaSpeed,
		m_TeleportLaserMinDistance,
		m_TeleportLaserMaxDistance
	);
	m_Laser->SetLength(m_TeleportLaserCurrentDistance);
}

const FVector &UVRControllerLeft::GetTeleportPoint() const {
	return m_Laser->GetEndPoint();
}

void UVRControllerLeft::UpdateLaserPositionDirection(const bool ShouldLerp) {
	Super::UpdateLaserPositionDirection(ShouldLerp);
	if (m_TeleportPreviewMesh->IsVisible())
		m_TeleportPreviewMesh->SetWorldLocation(GetTeleportPoint());
}
