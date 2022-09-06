#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolManipulatorPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolManipulatorPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	void SetTextSelectEntity() const;
	void SetTextActionEntity() const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UOptionSelectorWidget *ModeSelector;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ManipulatorText;
};
