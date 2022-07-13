#pragma once

#include "ButtonWidget.h"
#include "ImageButtonWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UImageButtonWidget : public UButtonWidget {
	GENERATED_BODY()
public:
	explicit UImageButtonWidget(const FObjectInitializer &ObjectInitializer);
	virtual void NativePreConstruct() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor ImageColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	UTexture2D *Image;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage *IBImage;
};
