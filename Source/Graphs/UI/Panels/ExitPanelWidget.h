#pragma once

#include "Blueprint/UserWidget.h"
#include "ExitPanelWidget.generated.h"

UCLASS(Abstract)
class GRAPHS_API UExitPanelWidget final : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextButtonWidget *ExitButton;
};
