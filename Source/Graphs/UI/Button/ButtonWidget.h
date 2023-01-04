#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ButtonWidget.generated.h"

/**
 * UButtonWidget is a widget that displays a button that can be clicked and perform an action.
 * It uses the standard UButton implementation, adjusting its visual component to match the uniform UI style of the
 * application.
 * 
 * It has several customizable properties such as background color, width and height overrides, and content padding.
 * It also has the ability to set a function to be called when the button is clicked.
 *
 * @note The native tick for this class is disabled.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
class GRAPHS_API UButtonWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	/**
	 * Sets the background color of the button and updates the hovered and pressed colors based on coefficients.
	 * 
	 * @param NewColor The new background color of the button.
	 */
	void SetBackgroundColor(const FLinearColor &NewColor);

	/**
	 * Sets the coefficients for the hovered and pressed states of the button.
	 * These coefficients are used to multiply the background color to get the hovered and pressed colors.
	 * 
	 * @param HoveredCoeff The coefficient to use for the hovered state.
	 * @param PressedCoeff The coefficient to use for the pressed state.
	 */
	void SetHoveredPressedCoefficients(float HoveredCoeff, float PressedCoeff);

	/**
	 * Sets the function to be called when the button is clicked.
	 * 
	 * @param ClickEvent The function to be called when the button is clicked.
	 */
	FORCEINLINE void SetOnClickEvent(TFunction<void()> &&ClickEvent) { OnClick = MoveTemp(ClickEvent); }

	/** The default background color for the button (to define a uniform UI style). */
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
