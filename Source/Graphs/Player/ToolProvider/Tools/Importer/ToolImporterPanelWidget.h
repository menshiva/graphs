#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolImporterPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolImporterPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void SetInputFiles(TArray<FString> &InputFilesPaths) const;

	void ShowImportPanel() const;
	void ShowSuccessPanel() const;
	void ShowErrorPanel(const FString &ErrorMessage) const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *ImporterPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UListView *ImporterList;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ImporterRefreshButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ImporterText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *ImporterConfirmButton;
};
