#pragma once

#include "ToolCreator.h"
#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolCreatorPanelWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolCreatorPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void UpdateModeViaSelector(CreationMode NewMode) const;
	void Update(const UToolCreator *CreatorTool) const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UOptionSelectorWidget *ModeSelector;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *CancelButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock *CreatorText;
};
