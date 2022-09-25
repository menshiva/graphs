#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ButtonWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
class GRAPHS_API UButtonWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	void SetBackgroundColor(const FLinearColor &NewColor);
	void SetHoveredPressedCoefficients(float HoveredCoeff, float PressedCoeff);

	void SetOnClickEvent(TFunction<void()> &&ClickEvent);

	constexpr static FLinearColor DefaultBackgroundColor = FLinearColor(0.036458f, 0.036458f, 0.036458f);
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
	FLinearColor BackgroundColor = DefaultBackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FMargin ContentPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HoveredCoefficient = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PressedCoefficient = 1.0f;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class USizeBox *BSizeBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton *BButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UScaleBox *BScaleBox;
private:
	UFUNCTION()
	void HandleClicked();

	TFunction<void()> OnClick;
};
