#include "VRControllerLeft.h"
#include "NiagaraComponent.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Utils/Utils.h"

DECLARE_CYCLE_STAT(TEXT("UVRControllerLeft::Tick"), STAT_UVRControllerLeft_Tick, STATGROUP_GRAPHS_PERF);

UVRControllerLeft::UVRControllerLeft(
	const FObjectInitializer &ObjectInitializer
) : UVRControllerBase(ObjectInitializer, EControllerHand::Left) {
	SetLaserColor(ColorConsts::GreenColor.ReinterpretAsLinear());
	SetLaserMinLength(15.0f);
	SetLaserLength(TeleportLaserDefaultLength);
	UVRControllerBase::SetLaserActive(false);

	const ConstructorHelpers::FObjectFinder<UStaticMesh> TeleportPreviewMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	const ConstructorHelpers::FObjectFinder<UMaterial> TeleportPreviewMaterialAsset(
		TEXT("/Game/Graphs/Materials/TeleportPreviewMaterial")
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
	TeleportPreviewMaterialInst->SetVectorParameterValue("Color", ColorConsts::GreenColor.ReinterpretAsLinear());
	TeleportPreviewMesh->SetMaterial(0, TeleportPreviewMaterialInst);
	TeleportPreviewMesh->SetupAttachment(this);

	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> TeleportRingAsset(TEXT("/Game/Graphs/VFX/TeleportRing"));
	TeleportRing = ObjectInitializer.CreateDefaultSubobject<UNiagaraComponent>(this, "TeleportRing");
	TeleportRing->SetAsset(TeleportRingAsset.Object);
	TeleportRing->SetColorParameter("User.CustomColor", ColorConsts::GreenColor.ReinterpretAsLinear());
	TeleportRing->Deactivate();
	TeleportRing->SetVisibility(false);
	TeleportRing->SetupAttachment(this);
}

void UVRControllerLeft::SetupInputBindings(UInputComponent *Pic) {
	BindAction(Pic, "LeftMenu", IE_Pressed, [this] {
		GetVrPawn()->ToggleMenu();
		PlayActionHapticEffect();
	});

	BindAction(Pic, "LeftTrigger", IE_Pressed, [this] {
		if (IsLaserActive()) {
			GetVrPawn()->Teleport(GetLaserEndPosition());
			PlayActionHapticEffect();
		}
	});

	BindAction(Pic, "LeftGrip", IE_Pressed, [this] {
		SetLaserActive(true);
		TeleportRing->Activate();
		TeleportRing->SetVisibility(true);
		TeleportPreviewMesh->SetVisibility(true);
		PlayActionHapticEffect();
	});
	BindAction(Pic, "LeftGrip", IE_Released, [this] {
		SetLaserActive(false);
		TeleportRing->Deactivate();
		TeleportRing->SetVisibility(false);
		TeleportPreviewMesh->SetVisibility(false);
	});

	BindAxis(Pic, "LeftThumbstickAxisY", [this] (const float Value) {
		if (IsLaserActive())
			SetLaserLengthDelta(Value);
	});
	BindAxis(Pic, "LeftThumbstickAxisX", [this] (const float Value) {
		static bool isClicked = false;
		if (fabsf(Value) >= 0.7f) {
			if (!isClicked) {
				isClicked = true;
				if (Value > 0.0f) GetVrPawn()->Rotate(1.0f);
				else GetVrPawn()->Rotate(-1.0f);
				PlayActionHapticEffect();
			}
		}
		else if (isClicked)
			isClicked = false;
	});
}

void UVRControllerLeft::TickComponent(
	const float DeltaTime,
	const ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
) {
	SCOPE_CYCLE_COUNTER(STAT_UVRControllerLeft_Tick);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsLaserActive()) {
		auto TeleportLocation = GetLaserEndPosition();
		TeleportPreviewMesh->SetWorldLocation(TeleportLocation);
		TeleportLocation.Z -= AVRPawn::GetHeight();
		TeleportRing->SetWorldLocation(TeleportLocation);
	}
}
