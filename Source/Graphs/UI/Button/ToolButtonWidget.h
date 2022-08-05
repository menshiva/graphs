#pragma once

#include "Blueprint/UserWidget.h"
#include "ToolButtonWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolButtonWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	void SetToolImage(UTexture2D *NewImage) const;
	void SetToolNameText(const FText &NewText) const;

	void SetOnClickEvent(TFunction<void()> &&ClickEvent) const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	UTexture2D *ToolImage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DesignerRebuild))
	FText ToolName;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UImageButtonWidget *TBButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *TBText;
};
