#pragma once

#include "Blueprint/UserWidget.h"
#include "ToolManipulatorPanelWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UToolManipulatorPanelWidget : public UUserWidget {
	GENERATED_BODY()
public:
	void SetTextSelectEntity() const;
	void SetTextMoveEntity() const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *ManipulatorText;
};
