#pragma once

#include "Blueprint/UserWidget.h"
#include "OptionSelectorWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UOptionSelectorWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	void SetOptions(std::initializer_list<FString> NewOptions);
	void SetSelectedOptionIndex(int32 NewIdx, bool TriggerEvent);
	void SelectPreviousOption(bool TriggerEvent);
	void SelectNextOption(bool TriggerEvent);
	void SetButtonsEnabled(bool Enabled);

	void SetOnSelectedOptionChangedEvent(TFunction<void(int32)> &&Event);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	TArray<FString> Options;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	int32 SelectedOptionIdx = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	bool ButtonsEnabled = true;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *SelectorPrevButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *OptionText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *SelectorNextButton;
private:
	TFunction<void(int32)> OnSelectedOptionChanged;
};
