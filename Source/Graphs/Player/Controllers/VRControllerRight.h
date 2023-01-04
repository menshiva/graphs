#pragma once

#include "VRControllerBase.h"
#include "VRControllerRight.generated.h"

enum class ControllerState : uint8_t {
	NONE,
	UI,
	TOOL
};

enum class SelectionMode : uint8_t {
	VERTEX_EDGE,
	GRAPH
};

class RightControllerInputInterface {
public:
	RightControllerInputInterface() = default;
	virtual ~RightControllerInputInterface() = default;

	virtual bool OnRightTriggerAction(bool IsPressed) { return false; }

	virtual bool OnRightThumbstickY(float Value) { return false; }

	virtual bool OnRightThumbstickX(float Value) { return false; }

	// Value is +1.0f on right action and -1.0f on left action
	virtual bool OnRightThumbstickXAction(float Value) { return false; }
};

UCLASS()
class GRAPHS_API UVRControllerRight final : public UVRControllerBase, public RightControllerInputInterface {
	GENERATED_BODY()
public:
	explicit UVRControllerRight(const FObjectInitializer &ObjectInitializer);

	virtual void SetupInputBindings(UInputComponent *Pic) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
	virtual bool OnRightThumbstickX(float Value) override;
	virtual bool OnRightThumbstickXAction(float Value) override;

	virtual void SetLaserActive(bool IsActive) override;
	void SetCastEnabled(bool Enable);

	void SetUiInteractionEnabled(bool Enabled);

	FORCEINLINE bool IsInUiState() const { return State == ControllerState::UI; }
	FORCEINLINE bool IsInToolState() const { return State == ControllerState::TOOL; }
	void SetToolStateEnabled(bool Enabled);

	FORCEINLINE SelectionMode GetSelectionMode() const { return Selection; }

	FORCEINLINE class UWidgetInteractionComponent *GetUiInteractor() const { return UiInteractor; }
protected:
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void OnUiHover(class UWidgetComponent *WidgetComponent, UWidgetComponent *PreviousWidgetComponent);

	UPROPERTY()
	UWidgetInteractionComponent *UiInteractor;

	UPROPERTY()
	UWidgetComponent *SelectionWidgetComponent;

	ControllerState State = ControllerState::NONE;

	bool TriggerPressed = false;
	bool LaserVisibleFlag = true;
	bool CastEnabled = true;

	SelectionMode Selection = SelectionMode::VERTEX_EDGE;

	constexpr static float MeshInteractionLaserMaxDistance = 15000.0f;
};
