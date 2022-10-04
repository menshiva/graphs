#include "OptionSelectorWidget.h"
#include "Components/TextBlock.h"
#include "Graphs/UI/Button/ButtonWidget.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UOptionSelectorWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (SelectorPrevButton)
		SelectorPrevButton->SetOnClickEvent([&] { SelectPreviousOption(true); });
	if (SelectorNextButton)
		SelectorNextButton->SetOnClickEvent([&] { SelectNextOption(true); });

	SetSelectedOptionIndex(SelectedOptionIdx, false);
	SetButtonsEnabled(ButtonsEnabled);
}

void UOptionSelectorWidget::SetOptions(std::initializer_list<FString> &&NewOptions) {
	Options.Empty(NewOptions.size());
	for (const auto &Item : NewOptions)
		Options.Push(Item);
	SetSelectedOptionIndex(SelectedOptionIdx, false);
}

void UOptionSelectorWidget::SetSelectedOptionIndex(const int32 NewIdx, const bool TriggerEvent) {
	SelectedOptionIdx = NewIdx;
	if (OptionText)
		OptionText->SetText(NewIdx >= 0 && NewIdx < Options.Num() ? FText::FromString(Options[NewIdx]) : FText::GetEmpty());
	if (TriggerEvent && OnSelectedOptionChanged)
		OnSelectedOptionChanged(NewIdx);
}

void UOptionSelectorWidget::SelectPreviousOption(const bool TriggerEvent) {
	if (Options.Num() > 0) {
		if (SelectedOptionIdx == 0)
			SelectedOptionIdx = Options.Num();
		SetSelectedOptionIndex(SelectedOptionIdx - 1, TriggerEvent);
	}
}

void UOptionSelectorWidget::SelectNextOption(const bool TriggerEvent) {
	if (Options.Num() > 0)
		SetSelectedOptionIndex((SelectedOptionIdx + 1) % Options.Num(), TriggerEvent);
}

void UOptionSelectorWidget::SetButtonsEnabled(const bool Enabled) {
	ButtonsEnabled = Enabled;

	const auto NewColor = Enabled
		? UButtonWidget::DefaultBackgroundColor
		: FLinearColor::Transparent;

	if (SelectorPrevButton) {
		if (Enabled)
			SelectorPrevButton->SetHoveredPressedCoefficients(0.8f, 0.6f);
		else
			SelectorPrevButton->SetHoveredPressedCoefficients(0.0f, 0.0f);
		SelectorPrevButton->SetBackgroundColor(NewColor);
	}
	if (SelectorNextButton) {
		if (Enabled)
			SelectorNextButton->SetHoveredPressedCoefficients(0.8f, 0.6f);
		else
			SelectorNextButton->SetHoveredPressedCoefficients(0.0f, 0.0f);
		SelectorNextButton->SetBackgroundColor(NewColor);
	}
}
