#pragma once

#include "../Controllers/VRControllerLeft.h"
#include "../Controllers/VRControllerRight.h"
#include "VRPawn.generated.h"

UCLASS()
class GRAPHS_API AVRPawn final : public APawn, public LeftControllerInputInterface, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerController *GetPlayerController() const;

	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);

	UFUNCTION()
	void QuitGame();

	virtual void OnLeftStateChanged(ControllerState NewState) override;

	virtual bool OnLeftMenuPressed() override;

	virtual bool OnLeftTriggerAction(bool IsPressed) override;
	virtual bool OnLeftGripAction(bool IsPressed) override;

	virtual bool OnLeftThumbstickYAction(float Value) override;
	virtual bool OnLeftThumbstickXAction(float Value) override;

	virtual bool OnLeftThumbstickYAxis(float Value) override;
	virtual bool OnLeftThumbstickXAxis(float Value) override;

	virtual void OnRightStateChanged(ControllerState NewState) override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual bool OnRightThumbstickYAction(float Value) override;
	virtual bool OnRightThumbstickXAction(float Value) override;

	virtual bool OnRightThumbstickYAxis(float Value) override;
	virtual bool OnRightThumbstickXAxis(float Value) override;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	constexpr static float Height = 111.0f;
protected:
	virtual void BeginPlay() override;
private:
	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float ToValue) const;

	UPROPERTY()
	class UCameraComponent *Camera;

	UPROPERTY()
	class UMenuWidgetComponent *Menu;

	bool IsCameraFadeAnimationRunning = false;

	constexpr static float RotationAngle = 45.0f;
	constexpr static float ScreenFadeDuration = 0.1f;
};
