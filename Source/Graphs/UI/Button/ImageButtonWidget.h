#pragma once

#include "ButtonWidget.h"
#include "ImageButtonWidget.generated.h"

/**
 * This is a class derived from UButtonWidget that represents a button with an image on it.
 * It has additional methods to set the image and image color of the button, and to set the visibility of the image.
 *
 * @note The native tick for this class is disabled.
 */
UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UImageButtonWidget : public UButtonWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	FORCEINLINE const FLinearColor &GetImageColor() const { return ImageColor; }

	void SetImage(UTexture2D *NewImage) const;
	void SetImageVisibility(bool Visible) const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor ImageColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	UTexture2D *Image;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage *IBImage;
};
