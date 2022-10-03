#pragma once

#include "../Tool.h"
#include "ToolManipulator.generated.h"

enum class ManipulationMode {
	MOVE,
	ROTATE
};

enum class RotationMode {
	Y_AXIS,
	Z_AXIS
};

UCLASS()
class GRAPHS_API UToolManipulator final : public UTool {
	GENERATED_BODY()
public:
	UToolManipulator();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void TickTool() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
	virtual bool OnRightThumbstickX(float Value) override;

	FORCEINLINE EntityId GetManipulationEntity() const { return ManipulationEntity; }

	FORCEINLINE ManipulationMode GetManipulationMode() const { return ManipMode; }
	void SetManipulationMode(ManipulationMode NewMode);

	FORCEINLINE RotationMode GetRotationMode() const { return RotMode; }
	FORCEINLINE void SetRotationMode(const RotationMode NewMode) { RotMode = NewMode; }
private:
	ManipulationMode ManipMode = ManipulationMode::MOVE;
	RotationMode RotMode = RotationMode::Y_AXIS;

	EntityId ManipulationEntity = EntityId::NONE();

	FVector PreviousLaserEndPosition;
	FVector GraphCenterPosition;

	float LastThumbstickYValue = 0.0f;
	float LastThumbstickXValue = 0.0f;

	constexpr static float DefaultRotationSpeed = 2.5f;
};
