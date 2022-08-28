#pragma once

#include "Blueprint/UserWidget.h"
#include "SelectorWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API USelectorWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	void SetItems(std::initializer_list<FString> NewItems);
	void SetSelectedItemIndex(int32 NewIdx, bool TriggerEvent);
	void SelectPreviousItem(bool TriggerEvent);
	void SelectNextItem(bool TriggerEvent);
	void SetButtonsEnabled(bool Enabled);

	void SetOnSelectedItemChangedEvent(TFunction<void(int32)> &&Event);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	TArray<FString> Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	int32 SelectedItemIdx = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	bool ButtonsEnabled = true;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *TBLeft;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *TBRight;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *Text;
private:
	TFunction<void(int32)> OnSelectedItemChanged;
};
