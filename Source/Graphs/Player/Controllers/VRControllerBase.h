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

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupPawn(class AVRPawn *Pawn);

	virtual void SetupInputBindings(UInputComponent *Pic) PURE_VIRTUAL(UVRControllerBase::SetupInputBindings);

	ControllerState GetState() const;
	virtual void SetState(ControllerState NewState);

	UMotionControllerComponent *GetMotionController() const;
	UMotionControllerComponent *GetMotionControllerAim() const;
	AVRPawn *GetVrPawn() const;

	const FVector &GetLaserPosition() const;
	const FVector &GetLaserDirection() const;
	bool IsLaserActive() const;
	void SetLaserActive(bool IsActive);
	void UpdateLaser(bool Lerp = true);

	const FHitResult &GetHitResult() const;
	void ResetHitResult();

	void PlayActionHapticEffect() const;

	bool TriggerPressed = false;
	bool GripPressed = false;

	float ThumbstickY = 0.0f;
	float ThumbstickX = 0.0f;
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

	TWeakObjectPtr<AVRPawn> VrPawn;

	UPROPERTY()
	UHapticFeedbackEffect_Base *HapticEffectController;

	UPROPERTY()
	UNiagaraComponent *Laser;

	EControllerHand Type;
	ControllerState State = ControllerState::NONE;

	FVector LaserPosition;
	FVector LaserDirection;
	float LaserLength = MeshInteractionLaserMaxDistance;

	FHitResult HitResult;

	constexpr static float ActionHapticScale = 0.15f;
	constexpr static FLinearColor MeshInteractionLaserColor = FLinearColor(0.033345f, 0.066689f, 0.161458f);
	constexpr static float MeshInteractionLaserMaxDistance = 5000.0f;
};
