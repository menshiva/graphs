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

	void PrimaryAction(bool IsPressed);
	void SecondaryAction(bool IsPressed);

	// -1.0f for turning left, +1.0f for turning right
	void Rotate(float Value);

	void SetTeleportationMode(bool Enabled);
	void AdjustTeleportDistance(float Delta);

	void ToggleMenu();
	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);

	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float Value) const;

	UPROPERTY()
	UCameraComponent *m_Camera;

	UPROPERTY()
	UVRControllerLeft *m_LeftController;

	UPROPERTY()
	UVRControllerRight *m_RightController;

	bool m_IsInTeleportationMode = false;
	bool m_IsCameraFadeAnimationRunning = false;

	constexpr static float m_ActionHapticScale = 0.3f;
	constexpr static float m_RotationAngle = 45.0f;
	constexpr static float m_ScreenFadeDuration = 0.1f;
};
