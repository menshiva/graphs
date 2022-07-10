#include "VRControllerBase.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const FString &Hand
) : USceneComponent(ObjectInitializer) {
	PrimaryComponentTick.bCanEverTick = true;

	m_MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this,
		"MotionController"
	);
	m_MotionController->SetShowDeviceModel(true);
	m_MotionController->SetTrackingMotionSource(FName(GetData(Hand)));
	m_MotionController->SetupAttachment(this);

	m_MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this,
		"MotionControllerAim"
	);
	m_MotionControllerAim->SetTrackingMotionSource(FName(GetData(Hand + "Aim")));
	m_MotionControllerAim->SetupAttachment(this);

	m_Laser = ObjectInitializer.CreateDefaultSubobject<ULaser>(this, "Laser");
	m_Laser->Init(m_MeshInteractionLaserColor, m_MeshInteractionLaserMaxDistance);
	m_Laser->SetupAttachment(m_MotionControllerAim);

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> ControllerActionHapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	m_ControllerActionHapticEffect = ControllerActionHapticEffectAsset.Object;
}

void UVRControllerBase::SetLaserVisibility(const bool IsVisible) const {
	m_Laser->SetVisibility(IsVisible);
}

void UVRControllerBase::UpdateLaserPositionDirection(const bool ShouldLerp) {
	if (ShouldLerp) {
		m_Laser->Update(m_AimLerpedPosition, m_AimLerpedDirection);
	}
	else {
		m_Laser->Update(
			m_MotionControllerAim->GetComponentLocation(),
			m_MotionControllerAim->GetForwardVector()
		);
	}
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction *ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	m_AimLerpedPosition = FMath::Lerp(m_AimLerpedPosition, m_MotionControllerAim->GetComponentLocation(), 0.5f);
	m_AimLerpedDirection = FMath::Lerp(m_AimLerpedDirection, m_MotionControllerAim->GetForwardVector(), 0.25f);
	UpdateLaserPositionDirection(true);

	// UWorld::LineTraceSingleByChannel()
	// ECC_GameTraceChannel2;
	/*FHitResult res;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		m_AimLerpedPosition,
		m_AimLerpedPosition + m_AimLerpedDirection * m_MeshInteractionLaserMaxDistance,
		TraceTypeQuery1,
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		res,
		true
	);*/
	/*UKismetSystemLibrary::LineTraceSingleByProfile(
		GetWorld(),
		m_AimLerpedPosition,
		m_AimLerpedPosition + m_AimLerpedDirection * m_MeshInteractionLaserMaxDistance,
		"BlockAll",
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		res,
		true
	);*/
}

void UVRControllerBase::AddActionBindingLambda(
	UInputComponent *PlayerInputComponent,
	const FName &ActionName,
	const EInputEvent InputEvent,
	const TFunction<void()> &Func
) {
	FInputActionBinding Action(ActionName, InputEvent);
	Action.ActionDelegate.GetDelegateForManualSet().BindLambda(Func);
	PlayerInputComponent->AddActionBinding(Action);
}
