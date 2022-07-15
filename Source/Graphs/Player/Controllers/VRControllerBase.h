#pragma once

#include "MotionControllerComponent.h"

enum class GRAPHS_API ControllerState {
	NONE,
	TELEPORTATION,
	UI,
	TOOL
};

class GRAPHS_API UVRControllerBase {
public:
	UVRControllerBase() = default;
	UVRControllerBase(
		const FObjectInitializer &ObjectInitializer,
		USceneComponent *aController,
		EControllerHand aControllerType
	);
	virtual ~UVRControllerBase() = default;

	void SetupPawn(class AVRPawn *Pawn);

	virtual void SetupInputBindings(UInputComponent *Pic) = 0;

	ControllerState GetState() const;
	virtual void SetState(ControllerState NewState);

	const FVector &GetLaserPosition() const;
	const FVector &GetLaserDirection() const;
	void SetLaserActive(bool IsActive) const;
	void UpdateLaser(bool Lerp = true);

	TWeakObjectPtr<UMotionControllerComponent> MotionController;
	TWeakObjectPtr<UMotionControllerComponent> MotionControllerAim;

	AVRPawn *VrPawn = nullptr;

	bool TriggerPressed = false;
	bool GripPressed = false;

	float ThumbstickY = 0.0f;
	float ThumbstickX = 0.0f;
protected:
	void PlayActionHapticEffect() const;
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
	TWeakObjectPtr<UHapticFeedbackEffect_Base> HapticEffectController;
	TWeakObjectPtr<UNiagaraComponent> Laser;

	EControllerHand Type;
	ControllerState State = ControllerState::NONE;

	FVector LaserPosition;
	FVector LaserDirection;

	constexpr static float ActionHapticScale = 0.15f;
	constexpr static FLinearColor MeshInteractionLaserColor = FLinearColor(0.07451f, 0.14902f, 0.360784f);
	constexpr static float MeshInteractionLaserMaxDistance = 5000.0f;
};
