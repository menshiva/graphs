#pragma once

#include "ButtonWidget.h"
#include "ImageButtonWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UImageButtonWidget final : public UButtonWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor ImageColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	UTexture2D *Image;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImage *IBImage;
};
