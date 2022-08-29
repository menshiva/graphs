#pragma once

#include "Graphs/Player/ToolProvider/Tools/Tool.h"
#include "ToolExporter.generated.h"

UCLASS()
class GRAPHS_API UToolExporter final : public UTool {
	GENERATED_BODY()
public:
	UToolExporter();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual bool OnRightTriggerAction(bool IsPressed) override;
};
