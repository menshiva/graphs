#pragma once

#include "VRControllerBase.h"
#include "VRControllerRight.generated.h"

class RightControllerInputInterface {
public:
	RightControllerInputInterface() = default;
	virtual ~RightControllerInputInterface() = default;

	virtual void OnRightStateChanged(ControllerState NewState) {}

	virtual bool OnRightTriggerAction(bool IsPressed) { return false; }

	virtual bool OnRightThumbstickYAction(float Value) { return false; }
	virtual bool OnRightThumbstickXAction(float Value) { return false; }

	virtual bool OnRightThumbstickYAxis(float Value) { return false; }
	virtual bool OnRightThumbstickXAxis(float Value) { return false; }
};

UCLASS()
class GRAPHS_API UVRControllerRight final : public USceneComponent, public UVRControllerBase, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UVRControllerRight(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void SetState(ControllerState NewState) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetUiInteractionEnabled(bool Enabled);
	void SetUiInteractorPointerKeyPressed(bool IsPressed, const FKey &Key) const;

	virtual void OnRightStateChanged(ControllerState NewState) override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual bool OnRightThumbstickYAction(float Value) override;
	virtual bool OnRightThumbstickXAction(float Value) override;

	virtual bool OnRightThumbstickYAxis(float Value) override;
	virtual bool OnRightThumbstickXAxis(float Value) override;
private:
	UFUNCTION()
	void OnUiHover(class UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent);

	UPROPERTY()
	class UWidgetInteractionComponent *UiInteractor;
};
