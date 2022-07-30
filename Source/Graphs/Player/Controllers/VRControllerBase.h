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
	void SetupVrPawn(AVRPawn *Pawn);

	bool IsLaserActive() const;
	virtual void SetLaserActive(bool IsActive);

	FORCEINLINE FVector GetLaserEndPosition() const { return LaserStartPosition + LaserLength * LaserDirection; }
	void ForceUpdateLaserTransform();

	void PlayActionHapticEffect() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;

	FORCEINLINE UMotionControllerComponent *GetMotionControllerAim() const { return MotionControllerAim; }

	static void BindAction(
		UInputComponent *PlayerInputComponent,
		const FName &ActionName,
		EInputEvent InputEvent,
		TFunction<void()> &&Func
	);
	static void BindAxis(
		UInputComponent *PlayerInputComponent,
		const FName &ActionName,
		TFunction<void(float)> &&Func
	);

	void SetLaserNiagaraColor(const FLinearColor &Color) const;
	FORCEINLINE float GetLaserLength() const { return LaserLength; }
	FORCEINLINE void SetLaserLength(const float NewLength) { LaserLength = NewLength; }
	FORCEINLINE const FVector &GetLaserStartPosition() const { return LaserStartPosition; }
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
	float LaserLength = 0;

	constexpr static float ActionHapticScale = 0.15f;
};
