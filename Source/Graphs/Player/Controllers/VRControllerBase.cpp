#include "VRControllerBase.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Player/ToolController/ToolController.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	EControllerHand aControllerType
) : Type(aControllerType) {
	PrimaryComponentTick.bCanEverTick = true;

	const auto UndType = static_cast<__underlying_type(EControllerHand)>(aControllerType);
	FString ControllerName = StaticEnum<EControllerHand>()->GetNameStringByValue(UndType);

	MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this,
		"MotionController"
	);
	MotionController->SetShowDeviceModel(true);
	MotionController->SetTrackingMotionSource(FName(GetData(ControllerName)));
	MotionController->SetupAttachment(this);

	MotionControllerAim = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(
		this,
		"MotionControllerAim"
	);
	MotionControllerAim->SetTrackingMotionSource(FName(GetData(ControllerName + "Aim")));
	MotionControllerAim->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> HapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	HapticEffectController = HapticEffectAsset.Object;

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	Laser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "Laser");
	Laser->SetAsset(LaserAsset.Object);
	Laser->SetColorParameter("User.CustomColor", MeshInteractionLaserColor);
	Laser->Deactivate();
	Laser->SetVisibility(false);
	Laser->SetupAttachment(MotionControllerAim);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Laser->IsVisible() && GetState() == ControllerState::NONE) {
		FHitResult NewHitResult;
		GetWorld()->LineTraceSingleByChannel(
			NewHitResult,
			LaserPosition,
			LaserPosition + LaserDirection * MeshInteractionLaserMaxDistance,
			ECC_GameTraceChannel2 // Graph trace channel
		);

		if (NewHitResult.GetActor() != HitResult.GetActor()) {
			if (HitResult.bBlockingHit && HitResult.GetActor() != nullptr) {
				// unhit previously hitted entity
				VrPawn->GetToolController()->OnEntityHitChanged(this, Cast<AEntity>(HitResult.GetActor()), false);
			}
			const auto NewHitEntity = Cast<AEntity>(NewHitResult.GetActor());
			if (NewHitResult.bBlockingHit && NewHitEntity != nullptr) {
				// hit newely hit entity
				VrPawn->GetToolController()->OnEntityHitChanged(this, NewHitEntity, true);
			}
		}

		HitResult = NewHitResult;
		if (HitResult.GetActor() != nullptr && Cast<AEntity>(HitResult.GetActor()) == nullptr)
			HitResult.Reset();

		if (HitResult.bBlockingHit && HitResult.GetActor() != nullptr)
			LaserLength = FVector::Dist(LaserPosition, HitResult.ImpactPoint);
		else
			LaserLength = MeshInteractionLaserMaxDistance;
	}

	UpdateLaser();
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

UMotionControllerComponent *UVRControllerBase::GetMotionController() const {
	return MotionController;
}

UMotionControllerComponent *UVRControllerBase::GetMotionControllerAim() const {
	return MotionControllerAim;
}

AVRPawn* UVRControllerBase::GetVrPawn() const {
	return VrPawn.Get();
}

const FVector &UVRControllerBase::GetLaserPosition() const {
	return LaserPosition;
}

const FVector& UVRControllerBase::GetLaserDirection() const {
	return LaserDirection;
}

bool UVRControllerBase::IsLaserActive() const {
	return Laser->IsVisible();
}

void UVRControllerBase::SetLaserActive(const bool IsActive) {
	if (IsActive) {
		SetLaserStartEnd(Laser, LaserPosition, LaserPosition + LaserDirection * LaserLength);
		Laser->Activate();
		Laser->SetVisibility(true);
	}
	else {
		Laser->Deactivate();
		Laser->SetVisibility(false);
		ResetHitResult();
	}
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

	if (Laser->IsVisible())
		SetLaserStartEnd(Laser, LaserPosition, LaserPosition + LaserDirection * LaserLength);
}

const FHitResult& UVRControllerBase::GetHitResult() const {
	return HitResult;
}

void UVRControllerBase::ResetHitResult() {
	if (HitResult.bBlockingHit && HitResult.GetActor() != nullptr)
		VrPawn->GetToolController()->OnEntityHitChanged(this, Cast<AEntity>(HitResult.GetActor()), false);
	HitResult.Reset();
}

void UVRControllerBase::PlayActionHapticEffect() const {
	VrPawn->GetPlayerController()->PlayHapticEffect(HapticEffectController, Type, ActionHapticScale);
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
