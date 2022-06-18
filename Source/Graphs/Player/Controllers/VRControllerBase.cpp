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
	Laser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "Laser");
	Laser->SetupAttachment(MotionControllerAim);
	Laser->SetAsset(LaserAsset.Object);

	static ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> ControllerActionHapticEffectAsset(TEXT(
		"/Game/Haptics/ControllerActionHapticEffect"
	));
	ControllerActionHapticEffect = ControllerActionHapticEffectAsset.Object;
}

void UVRControllerBase::PlayHapticEffect(APlayerController *PlayerController) const {
	PlayerController->PlayHapticEffect(ControllerActionHapticEffect, HandType);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const auto laserStart = MotionControllerAim->GetComponentLocation();
	const auto laserEnd = laserStart + MotionControllerAim->GetForwardVector() * LaserMaxDistance;

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		Laser,
		"User.PointArray", 0,
		laserStart, false
	);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		Laser,
		"User.PointArray", 1,
		laserEnd, false
	);
}
