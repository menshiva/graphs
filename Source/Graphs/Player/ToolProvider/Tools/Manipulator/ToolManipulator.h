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

	FORCEINLINE ManipulationMode GetMode() const { return Mode; }
	void SetMode(ManipulationMode NewMode);

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void TickTool() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
	virtual bool OnRightThumbstickX(float Value) override;
private:
	ManipulationMode Mode = ManipulationMode::MOVE;

	FVector PreviousLaserEndPosition;
	FVector GraphCenterPosition;

	constexpr static float DefaultRotationSpeed = 2.5f;
};
