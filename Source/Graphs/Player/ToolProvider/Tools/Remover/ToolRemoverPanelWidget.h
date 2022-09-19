#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolRemoverPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolRemoverPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	void SetButtonsEnabled(bool IsEnabled) const;
	FORCEINLINE void SetLoadingStatus(const bool Loading) { IsLoading = Loading; }
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *RemoverPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *RemoverRemoveButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *RemoverDeselectButton;
private:
	bool IsLoading = false;
};
