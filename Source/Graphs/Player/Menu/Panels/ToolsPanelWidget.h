#pragma once

#include "Blueprint/UserWidget.h"
#include "ToolsPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolsPanelWidget : public UUserWidget {
	GENERATED_BODY()
public:
	explicit UToolsPanelWidget(const FObjectInitializer &ObjectInitializer);

	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher *ToolPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UUniformGridPanel *ToolButtonsHolder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImageButtonWidget *CloseToolButton;
private:
	TWeakObjectPtr<class UToolProvider> ToolProvider;

	TSubclassOf<UUserWidget> ToolButtonWidgetClass;
};
