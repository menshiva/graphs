#pragma once

#include "MotionControllerComponent.h"
#include "NiagaraComponent.h"

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
		USceneComponent *Controller,
		const FString &Hand
	);
	virtual ~UVRControllerBase() = default;

	virtual void SetupInputBindings(APawn *Pawn, UInputComponent *Pic) const = 0;

	virtual void PlayHapticEffect(APlayerController *PlayerController, float Scale) = 0;

	virtual void SetState(ControllerState NewState);
	FORCEINLINE ControllerState GetState() const;

	FORCEINLINE const FVector &GetLaserStartPosition() const;
	FORCEINLINE const FVector &GetLaserEndPosition() const;
	FORCEINLINE const FVector &GetLaserDirection() const;
	FORCEINLINE void SetLaserColor(const FLinearColor &NewColor);
	FORCEINLINE void SetLaserLength(float NewLength);
	FORCEINLINE void SetLaserActive(bool IsActive) const;
	void UpdateLaser(bool Lerp = true);
protected:
	FORCEINLINE UMotionControllerComponent *GetMotionController() const;
	FORCEINLINE UMotionControllerComponent *GetMotionControllerAim() const;
	FORCEINLINE UHapticFeedbackEffect_Base *GetHapticEffectController() const;

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

	constexpr static FLinearColor m_MeshInteractionLaserColor = FLinearColor(0.07451f, 0.14902f, 0.360784f);
	constexpr static float m_MeshInteractionLaserMaxDistance = 5000.0f;
private:
	TWeakObjectPtr<UMotionControllerComponent> m_MotionController;
	TWeakObjectPtr<UMotionControllerComponent> m_MotionControllerAim;

	TWeakObjectPtr<UHapticFeedbackEffect_Base> m_HapticEffectController;
	TWeakObjectPtr<UNiagaraComponent> m_Laser;

	FVector m_LaserStartPosition;
	FVector m_LaserEndPosition;
	FVector m_LaserDirection;
	FLinearColor m_LaserColor = m_MeshInteractionLaserColor;
	float m_LaserLength = m_MeshInteractionLaserMaxDistance;

	ControllerState m_State = ControllerState::NONE;
};
