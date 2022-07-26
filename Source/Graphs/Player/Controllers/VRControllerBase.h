#pragma once

#include "MotionControllerComponent.h"
#include "VRControllerBase.generated.h"

enum class GRAPHS_API ControllerState {
	NONE,
	TELEPORTATION,
	UI,
	TOOL
};

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
	FORCEINLINE UMotionControllerComponent *GetMotionControllerAim() const { return MotionControllerAim; }

	FORCEINLINE class AVRPawn *GetVrPawn() const { return VrPawn.Get(); }
	void SetupVrPawn(AVRPawn *Pawn);

	FORCEINLINE const FVector &GetLaserStartPosition() const { return LaserStartPosition; }
	FORCEINLINE FVector GetLaserEndPosition() const { return LaserStartPosition + LaserLength * LaserDirection; }
	FORCEINLINE const FVector &GetLaserDirection() const { return LaserDirection; }

	FORCEINLINE bool IsLaserVisibleFlag() const { return LaserVisibleFlag; }
	void SetLaserActive(bool IsActive, bool UpdateFlag);
	void ForceUpdateLaserTransform();

	class AEntity *GetHitEntity() const;
	FORCEINLINE const FHitResult &GetHitResult() const { return HitResult; }
	void ResetHitResult();

	FORCEINLINE ControllerState GetState() const { return State; }
	virtual void SetState(const ControllerState NewState);

	void PlayActionHapticEffect() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
protected:
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
	static void SetLaserStartEnd(class UNiagaraComponent *aLaser, const FVector &Start, const FVector &End);
private:
	UPROPERTY()
	UMotionControllerComponent *MotionController;

	UPROPERTY()
	UMotionControllerComponent *MotionControllerAim;

	UPROPERTY()
	UNiagaraComponent *Laser;

	UPROPERTY()
	UHapticFeedbackEffect_Base *HapticEffectController;

	EControllerHand Type;
	ControllerState State = ControllerState::NONE;

	TWeakObjectPtr<AVRPawn> VrPawn;

	bool LaserVisibleFlag = false;
	FVector LaserStartPosition;
	FVector LaserDirection;
	float LaserLength = MeshInteractionLaserMaxDistance;

	FHitResult HitResult;

	constexpr static float ActionHapticScale = 0.15f;
	constexpr static float MeshInteractionLaserMaxDistance = 5000.0f;
};
