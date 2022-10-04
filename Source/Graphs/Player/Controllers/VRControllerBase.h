#pragma once

#include "MotionControllerComponent.h"
#include "VRControllerBase.generated.h"

UCLASS(Abstract)
class GRAPHS_API UVRControllerBase : public USceneComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(
		const FObjectInitializer &ObjectInitializer,
		EControllerHand aControllerType
	);

	virtual void SetupInputBindings(UInputComponent *Pic) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	FORCEINLINE UMotionControllerComponent *GetMotionController() const { return MotionController; }

	FORCEINLINE class AVRPawn *GetVrPawn() const { return VrPawn.Get(); }
	FORCEINLINE void SetupVrPawn(AVRPawn *Pawn) { VrPawn = Pawn; }

	bool IsLaserActive() const;
	virtual void SetLaserActive(bool IsActive);

	FORCEINLINE const FVector &GetLaserStartPosition() const { return LaserStartPosition; }
	FORCEINLINE FVector GetLaserEndPosition() const { return LaserStartPosition + LaserLength * LaserDirection; }
	FORCEINLINE float GetLaserLength() const { return LaserLength; }

	float SetLaserLength(const float NewLength);
	float SetLaserLengthDelta(const float Delta);
	void SetLaserColor(const FLinearColor &Color) const;
	void ForceUpdateLaserTransform();

	void PlayActionHapticEffect() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;

	FORCEINLINE UMotionControllerComponent *GetMotionControllerAim() const { return MotionControllerAim; }

	FORCEINLINE void SetLaserMinLength(const float NewMinLength) { LaserMinLength = NewMinLength; }
	FORCEINLINE const FVector &GetLaserDirection() const { return LaserDirection; }
private:
	static void SetLaserNiagaraStartEnd(class UNiagaraComponent *aLaser, const FVector &Start, const FVector &End);

	UPROPERTY()
	UMotionControllerComponent *MotionController;

	UPROPERTY()
	UMotionControllerComponent *MotionControllerAim;

	UPROPERTY()
	UNiagaraComponent *Laser;

	UPROPERTY()
	UHapticFeedbackEffect_Base *HapticEffectController;

	EControllerHand Type;

	TWeakObjectPtr<AVRPawn> VrPawn;

	FVector LaserStartPosition;
	FVector LaserDirection;
	float LaserMinLength = 0;
	float LaserLength = 0;

	constexpr static float ActionHapticScale = 0.15f;
	constexpr static float LaserMaxLength = 15000.0f;
	constexpr static float LaserLengthDeltaSpeed = 10.0f;
};
