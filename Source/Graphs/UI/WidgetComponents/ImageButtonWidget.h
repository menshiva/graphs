#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ImageButtonWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UImageButtonWidget final : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor BackgroundColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor ImageColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	UTexture2D *Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FMargin ImagePadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HoveredFloat = -0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PressedFloat = -0.8f;

	UPROPERTY(BlueprintAssignable, Category="Event")
	FOnButtonClickedEvent OnClicked;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UButton *IBButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UScaleBox *IBScaleBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage *IBImage;
private:
	UFUNCTION()
	void OnHovered();

	UFUNCTION()
	void OnUnhovered();

	UFUNCTION()
	void OnPressed();

	UFUNCTION()
	void OnReleased();

	FLinearColor AddFloatToBackgroundColor(float Value) const;
};
