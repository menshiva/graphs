#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolExporterPanelWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolExporterPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	void ShowExportPanel() const;
	void ShowSuccessPanel(const FString &ExportPath) const;
	void ShowErrorPanel(const FString &ErrorDesc) const;
	void ShowLoadingPanel() const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *ExporterPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ExporterText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ExporterConfirmButton;
};
