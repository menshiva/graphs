#pragma once

#include "Graphs/Player/Controllers/VRControllerLeft.h"
#include "Graphs/Player/Controllers/VRControllerRight.h"
#include "VRPawn.generated.h"

UCLASS(Config = UserPreferences)
class GRAPHS_API AVRPawn final : public APawn, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit AVRPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerController *GetPlayerController() const { return Cast<APlayerController>(Controller); }
	FORCEINLINE UVRControllerRight *GetRightVrController() const { return RightVrController; }
	FORCEINLINE class UToolProvider *GetToolProvider() const { return ToolProvider; }

	// https://docs.unrealengine.com/4.27/en-US/SharingAndReleasing/XRDevelopment/VR/VRPlatforms/SteamVR/HowTo/SeatedCamera/#:~:text=Z%20value%20to-,121,-.
	FORCEINLINE static float GetHeight() { return 121.0f; }

	void ToggleCameraFadeAnimation();
	FORCEINLINE bool IsCameraFadeAnimationEnabled() const { return CameraFadeAnimationEnabled; }

	void ToggleMenu() const;

	// -1.0f for left rotation, +1.0f for right rotation
	void Rotate(float Value);

	void Teleport(const FVector &Location);

	void QuitGame() const;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
protected:
	virtual void BeginPlay() override;
private:
	void CameraTeleportAnimation(TFunction<void()> &&DoAfterFadeIn);

	// 1.0f for FadeIn, 0.0f for FadeOut
	FORCEINLINE void FadeCamera(float ToValue) const;

	UPROPERTY()
	UVRControllerLeft *LeftVrController;

	UPROPERTY()
	UVRControllerRight *RightVrController;

	UPROPERTY()
	class UCameraComponent *Camera;

	UPROPERTY()
	class UMenuWidgetComponent *Menu;

	UPROPERTY()
	UToolProvider *ToolProvider;

	UPROPERTY(Config)
	bool CameraFadeAnimationEnabled = true;

	bool IsCameraFadeAnimationRunning = false;

	constexpr static float RotationAngle = 45.0f;
	constexpr static float ScreenFadeDuration = 0.1f;
};
