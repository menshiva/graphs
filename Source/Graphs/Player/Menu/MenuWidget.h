#pragma once

#include "Blueprint/UserWidget.h"
#include "MenuWidget.generated.h"

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UMenuWidget : public UUserWidget {
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	FORCEINLINE bool IsKeyboardVisible() const { return KeyboardVisible; }
	void SetKeyboardVisibility(bool Visible);

	void SetHitEntity(const char *EntityName, FString &&EntityCaption) const;
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UHorizontalBox *MenuButtonHolder;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UBorder *Background;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher *ActivePanelSwitcher;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock *HitEntityTitle;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock *HitEntityCaption;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UKeyboardWidget *Keyboard;
private:
	void SetActivePanel(size_t Index) const;

	constexpr static FLinearColor MenuButtonUnselectedColor = FLinearColor(0.03125f, 0.03125f, 0.03125f, 1.0f);

	bool KeyboardVisible = false;
};
