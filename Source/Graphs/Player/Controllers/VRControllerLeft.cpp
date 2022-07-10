#include "VRControllerLeft.h"
#include "../Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, "Left") {
	const ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Game/Graphs/Meshes/Capsule"));
	m_TeleportPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>("TeleportPreviewMesh");
	m_TeleportPreviewMesh->SetStaticMesh(TeleportPreviewMeshAsset.Object);
	m_TeleportPreviewMesh->SetVisibility(false);
	m_TeleportPreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	m_TeleportPreviewMesh->SetCastShadow(false);
	m_TeleportPreviewMesh->SetCastInsetShadow(false);
	m_TeleportPreviewMesh->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/TeleportPreviewMaterial")
	);
	const auto TeleportPreviewMaterialInst = m_TeleportPreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		TeleportPreviewMaterialAsset.Object
	);
	TeleportPreviewMaterialInst->SetVectorParameterValue("Color", m_TeleportLaserColor);
	m_TeleportPreviewMesh->SetMaterial(0, TeleportPreviewMaterialInst);

	const ConstructorHelpers::FClassFinder<UUserWidget> MainMenuAsset(TEXT("/Game/Graphs/UI/MainMenu"));
	m_MainMenuWidgetClass = MainMenuAsset.Class;
}

void UVRControllerLeft::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) const {
	const auto vrPawn = Cast<AVRPawn>(Pawn);

	PlayerInputComponent->BindAxis("LeftThumbstickAxisY", vrPawn, &AVRPawn::AdjustTeleportDistance);
	PlayerInputComponent->BindAxis("LeftThumbstickAxisX", vrPawn, &AVRPawn::Rotate);

	AddActionBindingLambda(
		PlayerInputComponent,
		"LeftTrigger", IE_Pressed,
		[vrPawn] {
			vrPawn->SecondaryAction(true);
		}
	);
	AddActionBindingLambda(
		PlayerInputComponent,
		"LeftTrigger", IE_Released,
		[vrPawn] {
			vrPawn->SecondaryAction(false);
		}
	);

	AddActionBindingLambda(
		PlayerInputComponent,
		"LeftGrip", IE_Pressed,
		[vrPawn] {
			vrPawn->SetTeleportationMode(true);
		}
	);
	AddActionBindingLambda(
		PlayerInputComponent,
		"LeftGrip", IE_Released,
		[vrPawn] {
			vrPawn->SetTeleportationMode(false);
		}
	);
}

void UVRControllerLeft::PlayHapticEffect(APlayerController *PlayerController, const float Scale) const {
	PlayerController->PlayHapticEffect(m_ControllerActionHapticEffect, EControllerHand::Left, Scale);
}

void UVRControllerLeft::UpdateLaserPositionDirection(const bool ShouldLerp) {
	Super::UpdateLaserPositionDirection(ShouldLerp);
	if (m_TeleportPreviewMesh->IsVisible())
		m_TeleportPreviewMesh->SetWorldLocation(GetTeleportPoint());
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

void UVRControllerLeft::SpawnMainMenu(APawn *Pawn) {
	m_MainMenu = NewObject<UWidgetComponent>(Pawn, UWidgetComponent::StaticClass(), "MainMenu");
	m_MainMenu->SetWidgetClass(m_MainMenuWidgetClass);
	m_MainMenu->SetDrawAtDesiredSize(true);
	m_MainMenu->SetPivot({0.0f, 0.5f});
	m_MainMenu->SetRelativeLocationAndRotation(
		FVector(0.0f, 5.0f, 0.0f),
		FRotator(75.0f, 15.0f, 185.0f)
	);
	m_MainMenu->SetRelativeScale3D(FVector(0.03f));
	m_MainMenu->SetGenerateOverlapEvents(false);
	m_MainMenu->CanCharacterStepUpOn = ECB_No;
	m_MainMenu->SetCollisionProfileName("VRUI");
	m_MainMenu->RegisterComponent();
	m_MainMenu->AttachToComponent(m_MotionController, FAttachmentTransformRules::KeepRelativeTransform);
}

void UVRControllerLeft::DestroyMainMenu() {
	m_MainMenu->DestroyComponent();
	m_MainMenu = nullptr;
}
