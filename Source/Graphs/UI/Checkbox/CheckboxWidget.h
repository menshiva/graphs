#pragma once

#include "Blueprint/UserWidget.h"
#include "CheckboxWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UCheckboxWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	bool IsTicked() const;
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
