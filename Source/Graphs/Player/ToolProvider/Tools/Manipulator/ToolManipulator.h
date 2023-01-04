#pragma once

#include "../Tool.h"
#include "ToolManipulator.generated.h"

enum class ManipulationMode {
	MOVE,
	ROTATE
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
private:
	ManipulationMode ManipMode = ManipulationMode::MOVE;

	EntityId ManipulationEntity = EntityId::NONE();

	FVector PreviousLaserEndPosition;
	FVector GraphCenterPosition;

	float LastThumbstickYValue = 0.0f;
	float LastThumbstickXValue = 0.0f;

	constexpr static float DefaultRotationSpeed = 2.5f;
};
