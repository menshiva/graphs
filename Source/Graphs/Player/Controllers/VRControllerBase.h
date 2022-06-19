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

	void ToggleMeshInteractionLaser(bool Enable);

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction *ThisTickFunction
	) override;
protected:
	FVector GetMotionControllerAimStartPos() const;
	FVector GetMotionControllerAimEndPos(float Distance) const;
	static void UpdateLaserPositions(UNiagaraComponent *Laser, const FVector &Start, const FVector &End);
private:
	EControllerHand HandType;

	UPROPERTY()
	UMotionControllerComponent *MotionController;

	UPROPERTY()
	UMotionControllerComponent *MotionControllerAim;

	UPROPERTY()
	UNiagaraComponent *MeshInteractionLaser;

	UPROPERTY()
	UHapticFeedbackEffect_Base *ControllerActionHapticEffect;

	bool MeshInteractionLaserVisibility = true;

	inline const static FColor MeshInteractionLaserColor = FColor::White;
	constexpr static float MeshInteractionLaserMaxDistance = 5000.0f;
};
