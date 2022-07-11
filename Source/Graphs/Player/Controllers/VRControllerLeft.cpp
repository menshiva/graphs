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
	m_MainMenu->AttachToComponent(GetMotionController(), FAttachmentTransformRules::KeepRelativeTransform);
}

void UVRControllerLeft::DestroyMainMenu() {
	m_MainMenu->DestroyComponent();
	m_MainMenu = nullptr;
}
