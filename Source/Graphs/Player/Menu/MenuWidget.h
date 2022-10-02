#pragma once

#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"

UCLASS(Abstract)
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UMenuWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder *Background;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher *ActivePanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UVerticalBox *MenuButtonHolder;
private:
	void SetActivePanel(size_t Index) const;

	constexpr static FLinearColor MenuButtonUnselectedColor = FLinearColor(0.03125f, 0.03125f, 0.03125f, 1.0f);
};
