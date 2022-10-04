#pragma once

#include "ButtonWidget.h"
#include "TextButtonWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UTextButtonWidget : public UButtonWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	FORCEINLINE const FString &GetTextStr() const { return Text.ToString(); }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	int32 FontSize = 24;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild, sRGB="true"))
	FLinearColor OutlineColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	int32 OutlineSize = 0;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *TBText;
};
