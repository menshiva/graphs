#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolExporterPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolExporterPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void SetTutorialStatus() const;
	void SetSuccessStatus(const FString &ExportedFileDir) const;
	void SetErrorStatus(const FString &ErrorDescription) const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher *ExporterStatusSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ExporterText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ExporterButton;
};
