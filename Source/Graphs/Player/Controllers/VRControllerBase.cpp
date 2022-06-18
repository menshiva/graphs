#include "VRControllerBase.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const FName &TrackingSource
) : UMotionControllerComponent(ObjectInitializer) {
	SetShowDeviceModel(true);
	SetTrackingMotionSource(TrackingSource);
	SetAssociatedPlayerIndex(0);

	// TODO: create laser and other subobjects like this:
	// MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(this, "GWSEYFERFYGREG");
	// MotionController->SetupAttachment(this);
}

void UVRControllerBase::SetupInputBindings(APawn *Pawn, UInputComponent *PlayerInputComponent) {}
