#pragma once

#include "Camera/CameraComponent.h"
#include "../Controllers/VRControllerLeft.h"
#include "../Controllers/VRControllerRight.h"
#include "VRPawn.generated.h"

UCLASS()
class GRAPHS_API AVRPawn final : public APawn {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerCameraManager *GetCameraManager() const;
	FORCEINLINE APlayerController *GetPlayerController() const;

	void PrimaryActionPressed();
	void PrimaryActionReleased();

	void SecondaryActionPressed();
	void SecondaryActionReleased();

	// -1.0f for turning left, +1.0f for turning right
	void Rotate(float Value);

	void AdjustTeleportDistance(float Delta);
	void TurnTeleportationModeOn();
	void TurnTeleportationModeOff();

	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);
	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float Value) const;

	UPROPERTY()
	UCameraComponent *Camera;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	bool IsInTeleportationMode = false;
	bool IsCameraFadeAnimationRunning = false;

	constexpr static float RotationAngle = 45.0f;
	constexpr static float ScreenFadeDuration = 0.15f;
};
