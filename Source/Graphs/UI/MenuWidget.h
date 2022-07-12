#pragma once

#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UMenuWidget final : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void PlayShowHideAnimation(EUMGSequencePlayMode::Type Mode, TFunction<void()> &&OnEnd);
protected:
	UPROPERTY(BlueprintReadOnly, Transient, meta=(BindWidgetAnim))
	UWidgetAnimation *ShowHideAnimation;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton *ExitButton;
};
