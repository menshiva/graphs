#pragma once

#include "Graphs/Player/ToolProvider/Tools/Tool.h"
#include "ToolManipulate.generated.h"

UCLASS()
class GRAPHS_API UToolManipulate final : public UTool {
	GENERATED_BODY()
public:
	UToolManipulate() = default;

	static FName GetToolName() {
		return "Editor";
	}

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void TickTool() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
	virtual bool OnRightThumbstickY(float Value) override;
private:
	FVector MovePosition;
};
