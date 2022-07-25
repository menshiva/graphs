#pragma once

#include "VRControllerBase.h"
#include "VRControllerRight.generated.h"

class RightControllerInputInterface {
public:
	RightControllerInputInterface() = default;
	virtual ~RightControllerInputInterface() = default;

	virtual bool OnRightTriggerAction(bool IsPressed) { return false; }
};

UCLASS()
class GRAPHS_API UVRControllerRight final : public UVRControllerBase, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UVRControllerRight(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;

	virtual void SetState(const ControllerState NewState) override;

	void SetUiInteractionEnabled(bool Enabled);
private:
	UFUNCTION()
	void OnUiHover(class UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent);

	UPROPERTY()
	class UWidgetInteractionComponent *UiInteractor;

	bool TriggerPressed = false;
};
