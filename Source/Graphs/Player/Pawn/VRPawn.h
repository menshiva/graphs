#pragma once

#include "Camera/CameraComponent.h"
#include "../../InputInterface.h"
#include "../Controllers/VRControllerLeft.h"
#include "../Controllers/VRControllerRight.h"
#include "../../UI/MenuWidgetComponent.h"
#include "VRPawn.generated.h"

UCLASS()
class GRAPHS_API AVRPawn final : public APawn, public InputInterface {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerController *GetPlayerController() const;

	virtual bool OnRightTriggerPressed() override;
	virtual bool OnRightTriggerReleased() override;

	virtual bool OnLeftTriggerPressed() override;
	virtual bool OnLeftTriggerReleased() override;

	virtual bool OnLeftGripPressed() override;
	virtual bool OnLeftGripReleased() override;

	virtual bool OnLeftThumbstickY(const float Value) override;
	virtual bool OnLeftThumbstickX(const float Value) override;
	virtual bool OnLeftThumbstickLeft() override;
	virtual bool OnLeftThumbstickRight() override;
	virtual bool OnLeftThumbstickUp() override;
	virtual bool OnLeftThumbstickDown() override;

	UFUNCTION(BlueprintCallable)
	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);

	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float ToValue) const;

	void ToggleMenu();

	UPROPERTY()
	UCameraComponent *m_Camera;

	UPROPERTY()
	UVRControllerLeft *m_LeftController;

	UPROPERTY()
	UVRControllerRight *m_RightController;

	UPROPERTY()
	UMenuWidgetComponent *m_Menu;

	bool m_IsCameraFadeAnimationRunning = false;

	constexpr static float m_ActionHapticScale = 0.15f;
	constexpr static float m_RotationAngle = 45.0f;
	constexpr static float m_ScreenFadeDuration = 0.1f;
};
