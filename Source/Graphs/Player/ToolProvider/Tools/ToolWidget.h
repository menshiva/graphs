#pragma once

#include "Tool.h"
#include "Graphs/Player/Menu/Panels/ToolsPanelWidget.h"
#include "ToolWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UToolWidget : public UUserWidget {
	GENERATED_BODY()
public:
	void Init(UToolsPanelWidget *ParentToolsPanel, UTool *CurrentTool) {
		ToolsPanel = ParentToolsPanel;
		Tool = CurrentTool;
	}
protected:
	template <class ToolClass>
	FORCEINLINE ToolClass *GetTool() const {
		return Cast<ToolClass>(Tool.Get());
	}

	FORCEINLINE void SetCloseToolButtonVisible(const bool Visible) const {
		ToolsPanel->SetCloseToolButtonVisible(Visible);
	}
private:
	TWeakObjectPtr<UToolsPanelWidget> ToolsPanel;
	TWeakObjectPtr<UTool> Tool;
};
