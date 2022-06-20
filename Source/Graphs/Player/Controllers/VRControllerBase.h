#pragma once

#include "MotionControllerComponent.h"
#include "Laser/Laser.h"
#include "VRControllerBase.generated.h"

UCLASS(Abstract, ClassGroup=(Custom))
class GRAPHS_API UVRControllerBase : public USceneComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(const FObjectInitializer &ObjectInitializer, EControllerHand Hand);

	virtual void SetupInputBindings(
		APawn *Pawn,
		UInputComponent *PlayerInputComponent
	) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	FORCEINLINE void PlayHapticEffect(APlayerController *PlayerController) const;

	FORCEINLINE void SetLaserVisibility(bool IsVisible) const;
	virtual void UpdateLaserPositionDirection(bool ShouldLerp);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
	UPROPERTY()
	ULaser *m_Laser;

	constexpr static FLinearColor m_MeshInteractionLaserColor = FLinearColor(0.07451f, 0.14902f, 0.360784f);
	constexpr static float m_MeshInteractionLaserMaxDistance = 5000.0f;
private:
	EControllerHand m_HandType;

	FVector m_AimPosition;
	FVector m_AimDirection;

	UPROPERTY()
	UMotionControllerComponent *m_MotionController;

	UPROPERTY()
	UMotionControllerComponent *m_MotionControllerAim;

	UPROPERTY()
	UHapticFeedbackEffect_Base *m_ControllerActionHapticEffect;
};
