#include "VRControllerBase.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const EControllerHand Hand
) : USceneComponent(ObjectInitializer), m_HandType(Hand) {
	PrimaryComponentTick.bCanEverTick = true;

	const auto handName = StaticEnum<EControllerHand>()->GetNameStringByValue(
		static_cast<__underlying_type(EControllerHand)>(Hand)
	);

	m_MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this, "MotionController"
	);
	m_MotionController->SetupAttachment(this);
	m_MotionController->SetShowDeviceModel(true);
	m_MotionController->SetTrackingMotionSource(FName(GetData(handName)));

	m_MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this, "MotionControllerAim"
	);
	m_MotionControllerAim->SetupAttachment(this);
	m_MotionControllerAim->SetTrackingMotionSource(FName(GetData(handName + "Aim")));

	m_Laser = ObjectInitializer.CreateDefaultSubobject<ULaser>(this, "Laser");
	m_Laser->SetupAttachment(m_MotionControllerAim);
	m_Laser->Init(m_MeshInteractionLaserColor, m_MeshInteractionLaserMaxDistance);

	static ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> ControllerActionHapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	m_ControllerActionHapticEffect = ControllerActionHapticEffectAsset.Object;
}

void UVRControllerBase::PlayHapticEffect(APlayerController *PlayerController) const {
	PlayerController->PlayHapticEffect(m_ControllerActionHapticEffect, m_HandType);
}

void UVRControllerBase::SetLaserVisibility(const bool IsVisible) const {
	m_Laser->SetVisibility(IsVisible);
}

void UVRControllerBase::UpdateLaserPositionDirection(const bool ShouldLerp) {
	if (ShouldLerp) {
		m_AimPosition = FMath::Lerp(m_AimPosition, m_MotionControllerAim->GetComponentLocation(), 0.5f);
		m_AimDirection = FMath::Lerp(m_AimDirection, m_MotionControllerAim->GetForwardVector(), 0.25f);
	}
	else {
		m_AimPosition = m_MotionControllerAim->GetComponentLocation();
		m_AimDirection = m_MotionControllerAim->GetForwardVector();
	}
	m_Laser->Update(m_AimPosition, m_AimDirection);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateLaserPositionDirection(true);
}
