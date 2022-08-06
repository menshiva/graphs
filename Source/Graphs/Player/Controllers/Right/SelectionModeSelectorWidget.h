#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SelectionModeSelectorWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API USelectionModeSelectorWidget : public UUserWidget {
	GENERATED_BODY()
public:
	FORCEINLINE void SetText(const char *Text) const { SelectionTypeText->SetText(FText::FromString(Text)); }
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock *SelectionTypeText;
};
