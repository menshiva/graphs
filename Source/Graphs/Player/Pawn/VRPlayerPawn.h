#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "../Controllers/VRControllerLeft.h"
#include "../Controllers/VRControllerRight.h"
#include "VRPlayerPawn.generated.h"

UCLASS()
class GRAPHS_API AVRPlayerPawn final : public APawn {
	GENERATED_BODY()
public:
	explicit AVRPlayerPawn(const FObjectInitializer &ObjectInitializer);
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;

	FORCEINLINE APlayerController *GetPlayerController() const;

	void TurnLeft();
	void TurnRight();
	void MoveY(float Speed);
	void MoveX(float Speed);
	void QuitGame();
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	USceneComponent *Root;

	UPROPERTY()
	UCameraComponent *Camera;

	UPROPERTY()
	UVRControllerLeft *LeftController;

	UPROPERTY()
	UVRControllerRight *RightController;

	constexpr static float SpeedCoefficient = 3.0f;
};
