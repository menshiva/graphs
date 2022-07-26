#include "VRControllerBase.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Graphs/Player/ToolController/ToolController.h"
#include "Graphs/Utils/Colors.h"

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

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> LaserAsset(TEXT("/Game/Graphs/VFX/LaserTrace"));
	Laser = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "Laser");
	Laser->SetAsset(LaserAsset.Object);
	Laser->SetColorParameter("User.CustomColor", ColorUtils::SelectionColor);
	if (!LaserVisibleFlag) {
		Laser->Deactivate();
		Laser->SetVisibility(false);
	}
	Laser->SetupAttachment(MotionControllerAim);
	ForceUpdateLaserTransform();

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> HapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	HapticEffectController = HapticEffectAsset.Object;
}

void UVRControllerBase::SetupVrPawn(AVRPawn *Pawn) {
	VrPawn = Pawn;
}

void UVRControllerBase::SetLaserActive(const bool IsActive, const bool UpdateFlag) {
	if (IsActive) {
		SetLaserStartEnd(Laser, LaserStartPosition, GetLaserEndPosition());
		Laser->Activate();
		Laser->SetVisibility(true);
	}
	else {
		Laser->Deactivate();
		Laser->SetVisibility(false);
		ResetHitResult();
	}
	if (UpdateFlag)
		LaserVisibleFlag = IsActive;
}

void UVRControllerBase::ForceUpdateLaserTransform() {
	LaserStartPosition = MotionControllerAim->GetComponentLocation();
	LaserDirection = MotionControllerAim->GetForwardVector();
}

AEntity *UVRControllerBase::GetHitEntity() const {
	if (HitResult.bBlockingHit)
		if (const auto Entity = Cast<AEntity>(HitResult.GetActor()))
			return Entity;
	return nullptr;
}

void UVRControllerBase::ResetHitResult() {
	if (const auto HitEntity = GetHitEntity())
		VrPawn->GetToolController()->OnEntityHitChanged(this, HitEntity, false);
	HitResult.Reset();
}

void UVRControllerBase::SetState(const ControllerState NewState) {
	State = NewState;
}

void UVRControllerBase::PlayActionHapticEffect() const {
	VrPawn->GetPlayerController()->PlayHapticEffect(HapticEffectController, Type, ActionHapticScale);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State == ControllerState::NONE && Laser->IsVisible()) {
		FHitResult NewHitResult;
		GetWorld()->LineTraceSingleByChannel(
			NewHitResult,
			LaserStartPosition,
			LaserStartPosition + MeshInteractionLaserMaxDistance * LaserDirection,
			ECC_GameTraceChannel2 // Graph trace channel
		);

		if (NewHitResult.GetActor() != HitResult.GetActor()) {
			if (const auto PrevHitEntity = GetHitEntity())
				VrPawn->GetToolController()->OnEntityHitChanged(this, PrevHitEntity, false);
			if (NewHitResult.bBlockingHit)
				if (const auto NewHitEntity = Cast<AEntity>(NewHitResult.GetActor()))
					VrPawn->GetToolController()->OnEntityHitChanged(this, NewHitEntity, true);
		}

		HitResult = NewHitResult;
		if (HitResult.GetActor() != nullptr && Cast<AEntity>(HitResult.GetActor()) == nullptr) {
			// make sure that hit actor is AEntity
			HitResult.Reset();
		}

		if (GetHitEntity() != nullptr)
			LaserLength = FVector::Dist(LaserStartPosition, HitResult.ImpactPoint);
		else
			LaserLength = MeshInteractionLaserMaxDistance;
	}

	LaserStartPosition = FMath::Lerp(LaserStartPosition, MotionControllerAim->GetComponentLocation(), 0.5f);
	LaserDirection = FMath::Lerp(LaserDirection, MotionControllerAim->GetForwardVector(), 0.25f);

	if (Laser->IsVisible())
		SetLaserStartEnd(Laser, LaserStartPosition, GetLaserEndPosition());
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
