#pragma once

#include "Graphs/Player/Controllers/VRControllerLeft.h"
#include "Graphs/Player/Controllers/VRControllerRight.h"
#include "Graphs/Provider/Entities/Entities.h"
#include "VRPawn.generated.h"

UCLASS(Config = UserPreferences)
class GRAPHS_API AVRPawn final : public APawn, public LeftControllerInputInterface, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	APlayerController *GetPlayerController() const;

	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);

	UFUNCTION()
	void ToggleCameraFadeAnimation();

	UFUNCTION()
	void QuitGame();

	virtual bool OnLeftMenuPressed() override;

	virtual bool OnLeftTriggerAction(bool IsPressed) override;
	virtual bool OnLeftGripAction(bool IsPressed) override;

	virtual bool OnLeftThumbstickYAction(float Value) override;
	virtual bool OnLeftThumbstickXAction(float Value) override;

	virtual bool OnLeftThumbstickYAxis(float Value) override;
	virtual bool OnLeftThumbstickXAxis(float Value) override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual bool OnRightThumbstickYAction(float Value) override;
	virtual bool OnRightThumbstickXAction(float Value) override;

	virtual bool OnRightThumbstickYAxis(float Value) override;
	virtual bool OnRightThumbstickXAxis(float Value) override;

	void OnEntityHitChanged(const UVRControllerBase *ControllerHit, const AEntity *Entity, bool IsHit) const;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	UPROPERTY(Config)
	bool CameraFadeAnimationEnabled = true;

	constexpr static float Height = 111.0f;
protected:
	virtual void BeginPlay() override;
private:
	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float ToValue) const;

	TWeakObjectPtr<class AGraphProvider> CachedProvider;

	UPROPERTY()
	class UCameraComponent *Camera;

	UPROPERTY()
	class UMenuWidgetComponent *Menu;

	bool IsCameraFadeAnimationRunning = false;

	constexpr static float RotationAngle = 45.0f;
	constexpr static float ScreenFadeDuration = 0.1f;
};
