#pragma once

#include "ButtonWidget.h"
#include "TextButtonWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UTextButtonWidget final : public UButtonWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor TextColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	int32 FontSize = 24;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *TBText;
};
