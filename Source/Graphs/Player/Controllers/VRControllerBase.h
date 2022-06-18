#pragma once

#include "MotionControllerComponent.h"
#include "NiagaraComponent.h"
#include "VRControllerBase.generated.h"

UCLASS(Abstract, ClassGroup=(Custom))
class GRAPHS_API UVRControllerBase : public USceneComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(
		const FObjectInitializer &ObjectInitializer,
		EControllerHand Hand
	);

	virtual void SetupInputBindings(
		APawn *Pawn,
		UInputComponent *PlayerInputComponent
	) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	void PlayHapticEffect(APlayerController *PlayerController) const;

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction *ThisTickFunction
	) override;
private:
	EControllerHand HandType;

	UPROPERTY()
	UMotionControllerComponent *MotionController;

	UPROPERTY()
	UMotionControllerComponent *MotionControllerAim;

	UPROPERTY()
	UNiagaraComponent *Laser;

	UPROPERTY()
	UHapticFeedbackEffect_Base *ControllerActionHapticEffect;

	constexpr static float LaserMaxDistance = 5000.0f;
};
