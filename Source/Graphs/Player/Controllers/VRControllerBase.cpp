#include "VRControllerBase.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include <NiagaraDataInterfaceArrayFunctionLibrary.h>

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const EControllerHand Hand
) : USceneComponent(ObjectInitializer), HandType(Hand) {
	PrimaryComponentTick.bCanEverTick = true;

	const auto handName = StaticEnum<EControllerHand>()->GetNameStringByValue(
		static_cast<__underlying_type(EControllerHand)>(Hand)
	);

	MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this, "MotionController"
	);
	MotionController->SetupAttachment(this);
	MotionController->SetShowDeviceModel(true);
	MotionController->SetTrackingMotionSource(FName(GetData(handName)));

	MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this, "MotionControllerAim"
	);
	MotionControllerAim->SetupAttachment(this);
	MotionControllerAim->SetTrackingMotionSource(FName(GetData(handName + "Aim")));

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/VFX/Laser"));
	MeshInteractionLaser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(
		this, "MeshInteractionLaser"
	);
	MeshInteractionLaser->SetupAttachment(this);
	MeshInteractionLaser->SetAsset(LaserAsset.Object);
	MeshInteractionLaser->SetVisibility(MeshInteractionLaserVisibility);
	MeshInteractionLaser->SetColorParameter("User.CustomColor", MeshInteractionLaserColor);

	static ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> ControllerActionHapticEffectAsset(TEXT(
		"/Game/Haptics/ControllerActionHapticEffect"
	));
	ControllerActionHapticEffect = ControllerActionHapticEffectAsset.Object;
}

void UVRControllerBase::PlayHapticEffect(APlayerController *PlayerController) const {
	PlayerController->PlayHapticEffect(ControllerActionHapticEffect, HandType);
}

void UVRControllerBase::ToggleMeshInteractionLaser(const bool Enable) {
	MeshInteractionLaserVisibility = Enable;
	MeshInteractionLaser->SetVisibility(Enable);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MeshInteractionLaserVisibility) {
		UpdateLaserPositions(
			MeshInteractionLaser,
			GetMotionControllerAimStartPos(),
			GetMotionControllerAimEndPos(MeshInteractionLaserMaxDistance)
		);
		// TODO: add lerp
	}
}

FVector UVRControllerBase::GetMotionControllerAimStartPos() const {
	return MotionControllerAim->GetComponentLocation();
}

FVector UVRControllerBase::GetMotionControllerAimEndPos(const float Distance) const {
	return GetMotionControllerAimStartPos() + MotionControllerAim->GetForwardVector() * Distance;
}

void UVRControllerBase::UpdateLaserPositions(UNiagaraComponent *Laser, const FVector &Start, const FVector &End) {
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		Laser,
		"User.PointArray", 0,
		Start, false
	);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		Laser,
		"User.PointArray", 1,
		End, false
	);
}
