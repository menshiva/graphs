#include "VRControllerBase.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Utils/Utils.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

DECLARE_CYCLE_STAT(TEXT("UVRControllerBase::Tick"), STAT_UVRControllerBase_Tick, STATGROUP_GRAPHS_PERF);

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const EControllerHand aControllerType
) : Type(aControllerType) {
	PrimaryComponentTick.bCanEverTick = true;

	FString ControllerName = StaticEnum<EControllerHand>()->GetNameStringByValue(Utils::EnumUnderlyingValue(aControllerType));

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
	Laser->Deactivate();
	Laser->SetVisibility(false);
	Laser->SetupAttachment(MotionControllerAim);

	const ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> HapticEffectAsset(TEXT(
		"/Game/Graphs/Haptics/ControllerActionHapticEffect"
	));
	HapticEffectController = HapticEffectAsset.Object;
}

void UVRControllerBase::SetupVrPawn(AVRPawn *Pawn) {
	VrPawn = Pawn;
}

bool UVRControllerBase::IsLaserActive() const {
	return Laser->IsVisible();
}

void UVRControllerBase::SetLaserActive(const bool IsActive) {
	if (IsActive) {
		ForceUpdateLaserTransform();
		SetLaserNiagaraStartEnd(Laser, LaserStartPosition, GetLaserEndPosition());
		Laser->Activate();
		Laser->SetVisibility(true);
	}
	else {
		Laser->Deactivate();
		Laser->SetVisibility(false);
	}
}

void UVRControllerBase::SetLaserLength(const float NewLength) {
	LaserLength = FMath::Clamp(NewLength, LaserMinLength, LaserMaxLength);
}

void UVRControllerBase::SetLaserLengthDelta(const float Delta) {
	LaserLength = FMath::Clamp(LaserLength + Delta * LaserLengthDeltaSpeed, LaserMinLength, LaserMaxLength);
}

void UVRControllerBase::SetLaserColor(const FLinearColor &Color) const {
	Laser->SetColorParameter("User.CustomColor", Color);
}

void UVRControllerBase::ForceUpdateLaserTransform() {
	LaserStartPosition = MotionControllerAim->GetComponentLocation();
	LaserDirection = MotionControllerAim->GetForwardVector();
}

void UVRControllerBase::PlayActionHapticEffect() const {
	VrPawn->GetController<APlayerController>()->PlayHapticEffect(HapticEffectController, Type, ActionHapticScale);
}

void UVRControllerBase::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SCOPE_CYCLE_COUNTER(STAT_UVRControllerBase_Tick);

	LaserStartPosition = FMath::Lerp(LaserStartPosition, MotionControllerAim->GetComponentLocation(), 0.5f);
	LaserDirection = FMath::Lerp(LaserDirection, MotionControllerAim->GetForwardVector(), 0.25f);

	if (IsLaserActive())
		SetLaserNiagaraStartEnd(Laser, LaserStartPosition, GetLaserEndPosition());
}

void UVRControllerBase::BeginPlay() {
	Super::BeginPlay();
	ForceUpdateLaserTransform();
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

void UVRControllerBase::SetLaserNiagaraStartEnd(UNiagaraComponent *aLaser, const FVector &Start, const FVector &End) {
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
