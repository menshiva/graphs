#pragma once

#include "MotionControllerComponent.h"
#include "Laser/Laser.h"
#include "VRControllerBase.generated.h"

UCLASS(Abstract, ClassGroup=(Custom))
class GRAPHS_API UVRControllerBase : public USceneComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(const FObjectInitializer &ObjectInitializer, const FString &Hand);

	virtual void SetupInputBindings(
		APawn *Pawn,
		UInputComponent *PlayerInputComponent
	) const PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	virtual void PlayHapticEffect(
		APlayerController *PlayerController,
		float Scale
	) const PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	FORCEINLINE void SetLaserVisibility(bool IsVisible) const;
	virtual void UpdateLaserPositionDirection(bool ShouldLerp);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
	static void AddActionBindingLambda(
		UInputComponent *PlayerInputComponent,
		const FName &ActionName,
		EInputEvent InputEvent,
		const TFunction<void()> &Func
	);

	UPROPERTY()
	ULaser *m_Laser;

	UPROPERTY()
	UMotionControllerComponent *m_MotionController;

	UPROPERTY()
	UMotionControllerComponent *m_MotionControllerAim;

	UPROPERTY()
	UHapticFeedbackEffect_Base *m_ControllerActionHapticEffect;

	constexpr static FLinearColor m_MeshInteractionLaserColor = FLinearColor(0.07451f, 0.14902f, 0.360784f);
	constexpr static float m_MeshInteractionLaserMaxDistance = 5000.0f;
private:
	EControllerHand m_HandType;

	FVector m_AimLerpedPosition;
	FVector m_AimLerpedDirection;
};
