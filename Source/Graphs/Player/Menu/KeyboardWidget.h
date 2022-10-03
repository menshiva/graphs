#pragma once

#include "Blueprint/UserWidget.h"
#include "KeyboardWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UKeyboardWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void PlayShowHideAnimation(EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd);
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *Button0;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button1;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button2;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button3;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button4;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button5;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button6;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button7;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button8;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *Button9;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *ButtonDel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextButtonWidget *ButtonDot;

	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetAnim))
	UWidgetAnimation *ShowHideAnimation;
};
