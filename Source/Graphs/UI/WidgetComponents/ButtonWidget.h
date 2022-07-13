#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ButtonWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UButtonWidget : public UUserWidget {
	GENERATED_BODY()
public:
	explicit UButtonWidget(const FObjectInitializer &ObjectInitializer);

	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetBackgroundColor(const FLinearColor &NewColor);

	UPROPERTY(BlueprintAssignable, Category="Event")
	FOnButtonClickedEvent OnClicked;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(editcondition="bOverride_WidthOverride", DesignerRebuild))
	float WidthOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(editcondition="bOverride_HeightOverride", DesignerRebuild))
	float HeightOverride;

	UPROPERTY(EditAnywhere, meta=(InlineEditConditionToggle, DesignerRebuild))
	uint8 bOverride_WidthOverride : 1;

	UPROPERTY(EditAnywhere, meta=(InlineEditConditionToggle, DesignerRebuild))
	uint8 bOverride_HeightOverride : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor BackgroundColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FMargin ContentPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HoveredCoefficient = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PressedCoefficient = 0.0f;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class USizeBox *BSizeBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton *BButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UScaleBox *BScaleBox;
private:
	UFUNCTION()
	void HandleClicked();
};
