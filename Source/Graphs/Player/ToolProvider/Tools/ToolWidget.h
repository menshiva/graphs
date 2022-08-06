#pragma once

#include "Blueprint/UserWidget.h"
#include "Tool.h"
#include "ToolWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UToolWidget : public UUserWidget {
	GENERATED_BODY()
public:
	FORCEINLINE void SetupTool(UTool *CurrentTool) { Tool = CurrentTool; }
protected:
	template <class ToolClass>
	ToolClass *GetTool() const { return Cast<ToolClass>(Tool.Get()); }
private:
	TWeakObjectPtr<UTool> Tool;
};
