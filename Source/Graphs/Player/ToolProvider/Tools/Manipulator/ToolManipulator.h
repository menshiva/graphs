#pragma once

#include "Graphs/Player/ToolProvider/Tools/Tool.h"
#include "ToolManipulator.generated.h"

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
private:
	FVector MovePosition;
};
