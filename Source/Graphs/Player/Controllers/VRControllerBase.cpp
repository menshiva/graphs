#include "VRControllerBase.h"
#include "Haptics/HapticFeedbackEffect_Base.h"

UVRControllerBase::UVRControllerBase(
	const FObjectInitializer &ObjectInitializer,
	const FName &TrackingSource
) : UMotionControllerComponent(ObjectInitializer) {
	SetShowDeviceModel(true);
	SetTrackingMotionSource(TrackingSource);
	SetAssociatedPlayerIndex(0);

	ControllerActionHapticEffect = ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base>(TEXT(
		"/Game/Controllers/ControllerActionHapticEffect"
	)).Object;

	// TODO: create laser and other subobjects like this:
	// MotionController = ObjectInitializer.CreateDefaultSubobject<UMotionControllerComponent>(this, "GWSEYFERFYGREG");
	// MotionController->SetupAttachment(this);
}
