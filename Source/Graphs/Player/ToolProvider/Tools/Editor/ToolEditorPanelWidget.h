#pragma once

#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "ToolEditorPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolEditorPanelWidget : public UToolWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	void Update();
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *EditorPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock *SelectedEntityTitle;

	// --------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UCheckboxWidget *ColorfulCheckbox;

	// --------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UGridPanel *ColorHolder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UWidgetSwitcher *ColorHolderPanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Pink;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Purple;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Cyan;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Teal;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Yellow;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Orange;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UImageButtonWidget *Brown;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *RandomColor;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *RandomizeVerticesColorsButton;

	// --------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UGridPanel *VertexValueHolder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UEditableText *VertexDataInput;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *CloseKeyboardBtn;

	// --------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *EditorSaveButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *EditorCancelButton;
private:
	UFUNCTION()
	void OnEditTextChanged(const FText &Text);

	UFUNCTION()
	void OnEditTextFocusLost(const FText &Text, ETextCommit::Type Type);

	void SetEntityColor(const class UToolEditor *EditorTool, const FLinearColor &Color);

	void SetDataChanged();

	TWeakObjectPtr<UMenuWidget> ParentMenu;

	bool IsDataChanged = false;
};
