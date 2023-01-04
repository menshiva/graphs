#pragma once

#include "Blueprint/UserWidget.h"
#include "CheckboxWidget.generated.h"

/**
 * UCheckboxWidget is a widget that displays an image button with a tick icon and a text next to it.
 * 
 * @note The native tick for this class is disabled.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UCheckboxWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	FORCEINLINE bool IsTicked() const { return Ticked; }
	void SetTicked(bool IsTicked);

	void SetOnClickEvent(TFunction<void()> &&ClickEvent);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	bool Ticked = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FText Text;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImageButtonWidget *CBButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *CBText;
};
