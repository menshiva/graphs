#include "VRControllerLeft.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Graphs/Player/Pawn/VRPawn.h"

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : USceneComponent(ObjectInitializer), UVRControllerBase(ObjectInitializer, this, EControllerHand::Left) {
	PrimaryComponentTick.bCanEverTick = true;

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	TeleportLaser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "TeleportLaser");
	TeleportLaser->SetComponentTickEnabled(false);
	TeleportLaser->SetAsset(LaserAsset.Object);
	TeleportLaser->SetColorParameter("User.CustomColor", TeleportLaserColor);
	TeleportLaser->Deactivate();
	TeleportLaser->SetVisibility(false);
	TeleportLaser->SetupAttachment(MotionControllerAim.Get());

	const ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	const ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/Pawn/TeleportPreviewMaterial")
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
	TeleportRing->SetComponentTickEnabled(false);
	TeleportRing->SetAsset(TeleportRingAsset.Object);
	TeleportRing->SetColorParameter("User.CustomColor", TeleportLaserColor);
	TeleportRing->Deactivate();
	TeleportRing->SetVisibility(false);
	TeleportRing->SetupAttachment(this);
}

void UVRControllerLeft::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "LeftMenu", IE_Pressed, [this] {
		OnLeftMenuPressed();
	});

	BindAction(Pic, "LeftTrigger", IE_Pressed, [this] {
		OnLeftTriggerAction(true);
	});
	BindAction(Pic, "LeftTrigger", IE_Released, [this] {
		OnLeftTriggerAction(false);
	});

	BindAction(Pic, "LeftGrip", IE_Pressed, [this] {
		OnLeftGripAction(true);
	});
	BindAction(Pic, "LeftGrip", IE_Released, [this] {
		OnLeftGripAction(false);
	});

	BindAxis(Pic, "LeftThumbstickAxisY", [this] (const float Value) {
		OnLeftThumbstickYAxis(Value);
	});
	BindAxis(Pic, "LeftThumbstickAxisX", [this] (const float Value) {
		OnLeftThumbstickXAxis(Value);
	});
}

void UVRControllerLeft::SetState(const ControllerState NewState) {
	OnLeftStateChanged(NewState);
	UVRControllerBase::SetState(NewState);
}

void UVRControllerLeft::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaser();
	if (GetState() == ControllerState::TELEPORTATION) {
		TeleportLocation = GetLaserPosition() + GetLaserDirection() * TeleportLaserCurrentDistance;
		SetLaserStartEnd(TeleportLaser, GetLaserPosition(), TeleportLocation);
		TeleportPreviewMesh->SetWorldLocation(TeleportLocation);
		TeleportLocation.Z -= VrPawn->Height;
		TeleportRing->SetWorldLocation(TeleportLocation);
	}
}

const FVector &UVRControllerLeft::GetTeleportLocation() const {
	return TeleportLocation;
}

void UVRControllerLeft::AdjustTeleportLaserLength(const float Delta) {
	TeleportLaserCurrentDistance = FMath::Clamp(
		TeleportLaserCurrentDistance + Delta * TeleportLaserLengthDeltaSpeed,
		TeleportLaserMinDistance,
		TeleportLaserMaxDistance
	);
}

void UVRControllerLeft::OnLeftStateChanged(const ControllerState NewState) {
	LeftControllerInputInterface::OnLeftStateChanged(NewState);
	SetLaserActive(NewState != ControllerState::TELEPORTATION);
	NewState == ControllerState::TELEPORTATION ? TeleportLaser->Activate() : TeleportLaser->Deactivate();
	TeleportLaser->SetVisibility(NewState == ControllerState::TELEPORTATION);
	TeleportPreviewMesh->SetVisibility(NewState == ControllerState::TELEPORTATION);
	NewState == ControllerState::TELEPORTATION ? TeleportRing->Activate() : TeleportRing->Deactivate();
	TeleportRing->SetVisibility(NewState == ControllerState::TELEPORTATION);
	VrPawn->OnLeftStateChanged(NewState);
}

bool UVRControllerLeft::OnLeftMenuPressed() {
	if (VrPawn->OnLeftMenuPressed()) {
		PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftMenuPressed();
}

bool UVRControllerLeft::OnLeftTriggerAction(const bool IsPressed) {
	TriggerPressed = IsPressed;
	if (VrPawn->OnLeftTriggerAction(IsPressed)) {
		if (IsPressed)
			PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftTriggerAction(IsPressed);
}

bool UVRControllerLeft::OnLeftGripAction(const bool IsPressed) {
	GripPressed = IsPressed;
	if (VrPawn->OnLeftGripAction(IsPressed)) {
		if (IsPressed)
			PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftGripAction(IsPressed);
}

bool UVRControllerLeft::OnLeftThumbstickYAction(const float Value) {
	if (VrPawn->OnLeftThumbstickYAction(Value)) {
		PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftThumbstickYAction(Value);
}

bool UVRControllerLeft::OnLeftThumbstickXAction(const float Value) {
	if (VrPawn->OnLeftThumbstickXAction(Value)) {
		PlayActionHapticEffect();
		return true;
	}
	return LeftControllerInputInterface::OnLeftThumbstickXAction(Value);
}

bool UVRControllerLeft::OnLeftThumbstickYAxis(const float Value) {
	ThumbstickY = Value;
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked) {
			isClicked = true;
			if (Value > 0.0f) OnLeftThumbstickYAction(1.0f);
			else OnLeftThumbstickYAction(-1.0f);
		}
	}
	else if (isClicked)
		isClicked = false;
	return VrPawn->OnLeftThumbstickYAxis(Value);
}

bool UVRControllerLeft::OnLeftThumbstickXAxis(const float Value) {
	ThumbstickX = Value;
	static bool isClicked = false;
	if (fabsf(Value) >= 0.7f) {
		if (!isClicked) {
			isClicked = true;
			if (Value > 0.0f) OnLeftThumbstickXAction(1.0f);
			else OnLeftThumbstickXAction(-1.0f);
		}
	}
	else if (isClicked)
		isClicked = false;
	return VrPawn->OnLeftThumbstickXAxis(Value);
}
