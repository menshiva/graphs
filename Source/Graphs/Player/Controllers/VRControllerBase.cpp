#include "VRControllerBase.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	USceneComponent *aController,
	EControllerHand aControllerType
) : Type(aControllerType) {
	const auto UndType = static_cast<__underlying_type(EControllerHand)>(aControllerType);
	FString ControllerName = StaticEnum<EControllerHand>()->GetNameStringByValue(UndType);

	MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		aController,
		"MotionController"
	);
	MotionController->SetShowDeviceModel(true);
	MotionController->SetTrackingMotionSource(FName(GetData(ControllerName)));
	MotionController->SetupAttachment(aController);

	MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		aController,
		"MotionControllerAim"
	);
	MotionControllerAim->SetTrackingMotionSource(FName(GetData(ControllerName + "Aim")));
	MotionControllerAim->SetupAttachment(aController);

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> HapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	HapticEffectController = HapticEffectAsset.Object;

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	Laser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(aController, "Laser");
	Laser->SetComponentTickEnabled(false);
	Laser->SetAsset(LaserAsset.Object);
	Laser->SetColorParameter("User.CustomColor", MeshInteractionLaserColor);
	Laser->SetupAttachment(MotionControllerAim.Get());
}

void UVRControllerBase::SetupPawn(AVRPawn *Pawn) {
	VrPawn = Pawn;
}

ControllerState UVRControllerBase::GetState() const {
	return State;
}

void UVRControllerBase::SetState(const ControllerState NewState) {
	State = NewState;
}

const FVector &UVRControllerBase::GetLaserPosition() const {
	return LaserPosition;
}

const FVector& UVRControllerBase::GetLaserDirection() const {
	return LaserDirection;
}

void UVRControllerBase::SetLaserActive(const bool IsActive) const {
	IsActive ? Laser->Activate() : Laser->Deactivate();
	Laser->SetVisibility(IsActive);
}

void UVRControllerBase::UpdateLaser(const bool Lerp) {
	if (Lerp) {
		LaserPosition = FMath::Lerp(LaserPosition, MotionControllerAim->GetComponentLocation(), 0.5f);
		LaserDirection = FMath::Lerp(LaserDirection, MotionControllerAim->GetForwardVector(), 0.25f);
	}
	else {
		LaserPosition = MotionControllerAim->GetComponentLocation();
		LaserDirection = MotionControllerAim->GetForwardVector();
	}

	if (Laser->IsVisible()) {
		SetLaserStartEnd(
			Laser.Get(),
			LaserPosition,
			LaserPosition + LaserDirection * MeshInteractionLaserMaxDistance
		);
	}
}

void UVRControllerBase::PlayActionHapticEffect() const {
	VrPawn->GetPlayerController()->PlayHapticEffect(HapticEffectController.Get(), Type, ActionHapticScale);
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

void UVRControllerBase::SetLaserStartEnd(UNiagaraComponent *aLaser, const FVector &Start, const FVector &End) {
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
			aLaser,
			"User.PointArray", 0,
			Start, false
		);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVectorValue(
		aLaser,
		"User.PointArray", 1,
		End, false
	);
}

// TODO: trace graph actors
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
