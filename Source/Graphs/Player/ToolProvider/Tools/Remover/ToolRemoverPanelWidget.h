#pragma once

#include "../ToolWidget.h"
#include "ToolRemoverPanelWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolRemoverPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void SetButtonsEnabled(bool IsEnabled) const;
	void SetLoadingStatus(const bool Loading) const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *RemoverPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *RemoverRemoveButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *RemoverDeselectButton;
};
