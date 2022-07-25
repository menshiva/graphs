#include "VRControllerLeft.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Graphs/Player/Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Left) {
	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	TeleportLaser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "TeleportLaser");
	TeleportLaser->SetAsset(LaserAsset.Object);
	TeleportLaser->SetColorParameter("User.CustomColor", TeleportLaserColor);
	TeleportLaser->Deactivate();
	TeleportLaser->SetVisibility(false);
	TeleportLaser->SetupAttachment(GetMotionControllerAim());

	const ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	const ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/TeleportPreviewMaterial")
	);
	TeleportPreviewMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(
		this, "TeleportPreviewMesh"
	);
	TeleportPreviewMesh->SetStaticMesh(TeleportPreviewMeshAsset.Object);
	TeleportPreviewMesh->SetVisibility(false);
	TeleportPreviewMesh->SetCollisionProfileName(TEXT("NoCollision"));
	TeleportPreviewMesh->SetCastShadow(false);
	TeleportPreviewMesh->SetRelativeScale3D(FVector(0.25f));
	const auto TeleportPreviewMaterialInst = TeleportPreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(
		0,
		TeleportPreviewMaterialAsset.Object
	);
	TeleportPreviewMaterialInst->SetVectorParameterValue("Color", TeleportLaserColor);
	TeleportPreviewMesh->SetMaterial(0, TeleportPreviewMaterialInst);
	TeleportPreviewMesh->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> TeleportRingAsset(TEXT("/Game/Graphs/VFX/TeleportRing"));
	TeleportRing = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "TeleportRing");
	TeleportRing->SetAsset(TeleportRingAsset.Object);
	TeleportRing->SetColorParameter("User.CustomColor", TeleportLaserColor);
	TeleportRing->Deactivate();
	TeleportRing->SetVisibility(false);
	TeleportRing->SetupAttachment(this);
}

void UVRControllerLeft::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "LeftMenu", IE_Pressed, [this] {
		GetVrPawn()->ToggleMenu();
		PlayActionHapticEffect();
	});

	BindAction(Pic, "LeftTrigger", IE_Pressed, [this] {
		OnLeftTriggerAction(true);
	});
	BindAction(Pic, "LeftTrigger", IE_Released, [this] {
		OnLeftTriggerAction(false);
	});

	BindAction(Pic, "LeftGrip", IE_Pressed, [this] {
		SetState(ControllerState::TELEPORTATION);
		if (IsLaserVisibleFlag())
			SetLaserActive(false, false);
		ResetHitResult();
		TeleportLaser->Activate();
		TeleportRing->Activate();
		TeleportLaser->SetVisibility(true);
		TeleportRing->SetVisibility(true);
		TeleportPreviewMesh->SetVisibility(true);
		PlayActionHapticEffect();
	});
	BindAction(Pic, "LeftGrip", IE_Released, [this] {
		SetState(ControllerState::NONE);
		if (IsLaserVisibleFlag())
			SetLaserActive(true, false);
		TeleportLaser->Deactivate();
		TeleportRing->Deactivate();
		TeleportLaser->SetVisibility(false);
		TeleportRing->SetVisibility(false);
		TeleportPreviewMesh->SetVisibility(false);
	});

	BindAxis(Pic, "LeftThumbstickAxisY", [this] (const float Value) {
		if (GetState() == ControllerState::TELEPORTATION) {
			TeleportLaserCurrentDistance = FMath::Clamp(
				TeleportLaserCurrentDistance + Value * TeleportLaserLengthDeltaSpeed,
				TeleportLaserMinDistance,
				TeleportLaserMaxDistance
			);
		}
	});
	BindAxis(Pic, "LeftThumbstickAxisX", [this] (const float Value) {
		static bool isClicked = false;
		if (fabsf(Value) >= 0.7f) {
			if (!isClicked) {
				isClicked = true;
				if (Value > 0.0f) GetVrPawn()->Rotate(1.0f);
				else GetVrPawn()->Rotate(-1.0f);
				PlayActionHapticEffect();
			}
		}
		else if (isClicked)
			isClicked = false;
	});
}

void UVRControllerLeft::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetState() == ControllerState::TELEPORTATION) {
		TeleportLocation = GetLaserStartPosition() + GetLaserDirection() * TeleportLaserCurrentDistance;
		SetLaserStartEnd(TeleportLaser, GetLaserStartPosition(), TeleportLocation);
		TeleportPreviewMesh->SetWorldLocation(TeleportLocation);
		TeleportRing->SetWorldLocation({TeleportLocation.X, TeleportLocation.Y, TeleportLocation.Z - AVRPawn::GetHeight()});
	}
}

bool UVRControllerLeft::OnLeftTriggerAction(const bool IsPressed) {
	if (IsPressed && GetState() == ControllerState::TELEPORTATION) {
		GetVrPawn()->Teleport(TeleportLocation);
		PlayActionHapticEffect();
		return true;
	}
	if (GetVrPawn()->OnLeftTriggerAction(IsPressed)) {
		if (IsPressed)
			PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftTriggerAction(IsPressed);
}
