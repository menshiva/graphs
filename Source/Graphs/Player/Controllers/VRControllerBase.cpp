#include "VRControllerBase.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	USceneComponent *Controller,
	const FString &Hand
) {
	m_MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		Controller,
		"MotionController"
	);
	m_MotionController->SetShowDeviceModel(true);
	m_MotionController->SetTrackingMotionSource(FName(GetData(Hand)));
	m_MotionController->SetupAttachment(Controller);

	m_MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		Controller,
		"MotionControllerAim"
	);
	m_MotionControllerAim->SetTrackingMotionSource(FName(GetData(Hand + "Aim")));
	m_MotionControllerAim->SetupAttachment(Controller);

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> ControllerActionHapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	m_HapticEffectController = ControllerActionHapticEffectAsset.Object;

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	m_Laser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(Controller, "Laser");
	m_Laser->SetComponentTickEnabled(false);
	m_Laser->SetAsset(LaserAsset.Object);
	m_Laser->SetupAttachment(m_MotionControllerAim.Get());
}

void UVRControllerBase::SetState(const ControllerState NewState) {
	m_State = NewState;
	SetLaserActive(NewState != ControllerState::UI && NewState != ControllerState::TOOL);
	UpdateLaser(false);
}

UMotionControllerComponent *UVRControllerBase::GetMotionController() const {
	return m_MotionController.Get();
}

UMotionControllerComponent *UVRControllerBase::GetMotionControllerAim() const {
	return m_MotionControllerAim.Get();
}

UHapticFeedbackEffect_Base *UVRControllerBase::GetHapticEffectController() const {
	return m_HapticEffectController.Get();
}

ControllerState UVRControllerBase::GetState() const {
	return m_State;
}

const FVector &UVRControllerBase::GetLaserStartPosition() const {
	return m_LaserStartPosition;
}

const FVector &UVRControllerBase::GetLaserEndPosition() const {
	return m_LaserEndPosition;
}

const FVector& UVRControllerBase::GetLaserDirection() const {
	return m_LaserDirection;
}

void UVRControllerBase::SetLaserColor(const FLinearColor& NewColor) {
	m_LaserColor = NewColor;
}

void UVRControllerBase::SetLaserLength(const float NewLength) {
	m_LaserLength = NewLength;
}

void UVRControllerBase::SetLaserActive(const bool IsActive) const {
	IsActive ? m_Laser->Activate() : m_Laser->Deactivate();
	m_Laser->SetVisibility(IsActive);
}

void UVRControllerBase::UpdateLaser(const bool Lerp) {
	if (!m_Laser->IsVisible()) return;
	if (Lerp) {
		m_LaserStartPosition = FMath::Lerp(m_LaserStartPosition, m_MotionControllerAim->GetComponentLocation(), 0.5f);
		m_LaserDirection = FMath::Lerp(m_LaserDirection, m_MotionControllerAim->GetForwardVector(), 0.25f);
	}
	else {
		m_LaserStartPosition = m_MotionControllerAim->GetComponentLocation();
		m_LaserDirection = m_MotionControllerAim->GetForwardVector();
	}
	m_LaserEndPosition = m_LaserStartPosition + m_LaserDirection * m_LaserLength;

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		m_Laser.Get(),
		"User.PointArray", 0,
		m_LaserStartPosition, false
	);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		m_Laser.Get(),
		"User.PointArray", 1,
		m_LaserEndPosition, false
	);
	m_Laser->SetColorParameter("User.CustomColor", m_LaserColor);
}

void UVRControllerBase::BindAction(
	UInputComponent *PlayerInputComponent,
	const FName& ActionName,
	const EInputEvent InputEvent,
	TFunction<void()> &&Func
) {
	FInputActionBinding AB(ActionName, InputEvent);
	AB.ActionDelegate.GetDelegateForManualSet().BindLambda(Func);
	PlayerInputComponent->AddActionBinding(MoveTemp(AB));
}

void UVRControllerBase::BindAxis(
	UInputComponent *PlayerInputComponent,
	const FName &ActionName,
	TFunction<void(float)> &&Func
) {
	FInputAxisBinding AB(ActionName);
	AB.AxisDelegate.GetDelegateForManualSet().BindLambda(Func);
	PlayerInputComponent->AxisBindings.Push(MoveTemp(AB));
}

/*void UVRControllerBase::Tick() {
	// UWorld::LineTraceSingleByChannel()
	// ECC_GameTraceChannel2;
	FHitResult res;
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
	);
	UKismetSystemLibrary::LineTraceSingleByProfile(
		GetWorld(),
		m_AimLerpedPosition,
		m_AimLerpedPosition + m_AimLerpedDirection * m_MeshInteractionLaserMaxDistance,
		"BlockAll",
		false,
		{},
		EDrawDebugTrace::ForOneFrame,
		res,
		true
	);
}*/
