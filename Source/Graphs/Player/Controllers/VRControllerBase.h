#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "VRControllerBase.generated.h"

UCLASS(Abstract, ClassGroup=(Custom))
class GRAPHS_API UVRControllerBase : public UMotionControllerComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(const FObjectInitializer &ObjectInitializer, const FName &TrackingSource);

	virtual void SetupInputBindings(
		APawn *Pawn,
		UInputComponent *PlayerInputComponent
	) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	virtual void PlayHapticEffect(
		APlayerController *PlayerController
	) const PURE_VIRTUAL(UVRControllerBase::PlayHapticEffect);
protected:
	UPROPERTY()
	UHapticFeedbackEffect_Base *ControllerActionHapticEffect;
};
