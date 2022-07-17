#pragma once

#include "MotionControllerComponent.h"
#include "VRControllerBase.generated.h"

enum class GRAPHS_API ControllerState {
	NONE,
	TELEPORTATION,
	UI,
	TOOL
};

UCLASS()
class GRAPHS_API UVRControllerBase : public USceneComponent {
	GENERATED_BODY()
public:
	UVRControllerBase() = default;
	UVRControllerBase(
		const FObjectInitializer &ObjectInitializer,
		EControllerHand aControllerType
	);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupPawn(class AVRPawn *Pawn);

	virtual void SetupInputBindings(UInputComponent *Pic) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	FORCEINLINE ControllerState GetState() const;
	virtual void SetState(ControllerState NewState);

	FORCEINLINE const FVector &GetLaserPosition() const;
	FORCEINLINE const FVector &GetLaserDirection() const;
	void SetLaserActive(bool IsActive) const;
	void UpdateLaser(bool Lerp = true);

	FORCEINLINE const FHitResult &GetHitResult() const;
	FORCEINLINE void ResetHitResult();

	UPROPERTY()
	UMotionControllerComponent *MotionController;

	UPROPERTY()
	UMotionControllerComponent *MotionControllerAim;

	TWeakObjectPtr<AVRPawn> VrPawn;

	bool TriggerPressed = false;
	bool GripPressed = false;

	float ThumbstickY = 0.0f;
	float ThumbstickX = 0.0f;
protected:
	FORCEINLINE void PlayActionHapticEffect() const;
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
	FORCEINLINE void TraceGraphComponents();

	UPROPERTY()
	UHapticFeedbackEffect_Base *HapticEffectController;

	UPROPERTY()
	UNiagaraComponent *Laser;

	EControllerHand Type;
	ControllerState State = ControllerState::NONE;

	FVector LaserPosition;
	FVector LaserDirection;

	FHitResult HitResult;

	constexpr static float ActionHapticScale = 0.15f;
	constexpr static FLinearColor MeshInteractionLaserColor = FLinearColor(0.07451f, 0.14902f, 0.360784f);
	constexpr static float MeshInteractionLaserMaxDistance = 5000.0f;
};
