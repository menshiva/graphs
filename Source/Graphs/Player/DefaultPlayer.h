#pragma once

#include "CoreMinimal.h"
#include "DefaultPlayer.generated.h"

UCLASS()
class GRAPHS_API ADefaultPlayer final : public APawn {
	GENERATED_BODY()
public:
	ADefaultPlayer();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void SetupPlayerInputComponent(UInputComponent *PlayerInputComponent) override;
private:
	USceneComponent *CreatePawn();
	void CreateHandController(USceneComponent *VrCameraRoot, const FName &ObjectName, const FName &HandType);

	// Left controller inputs
	void OnLeftControllerThumbstickLeft();
	void OnLeftControllerThumbstickRight();

	// Right controller inputs
	void OnRightControllerTriggerPressed();
	void OnRightControllerTriggerReleased();

	// Other inputs
	void OnKeyboardEscPressed();
};
