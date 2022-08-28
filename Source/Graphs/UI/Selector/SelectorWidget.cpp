#include "SelectorWidget.h"

#include "Components/TextBlock.h"
#include "Graphs/UI/Button/ButtonWidget.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void USelectorWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	if (TBLeft)
		TBLeft->SetOnClickEvent([&] { SelectPreviousItem(true); });
	if (TBRight)
		TBRight->SetOnClickEvent([&] { SelectNextItem(true); });

	SetSelectedItemIndex(SelectedItemIdx, false);
	SetButtonsEnabled(ButtonsEnabled);
}

void USelectorWidget::SetItems(const std::initializer_list<FString> NewItems) {
	Items.Empty(NewItems.size());
	for (const auto &Item : NewItems)
		Items.Push(Item);
	SetSelectedItemIndex(SelectedItemIdx, false);
}

void USelectorWidget::SetSelectedItemIndex(const int32 NewIdx, const bool TriggerEvent) {
	SelectedItemIdx = NewIdx;
	if (Text) {
		if (NewIdx < Items.Num())
			Text->SetText(FText::FromString(Items[NewIdx]));
		else
			Text->SetText(FText::GetEmpty());
	}
	if (TriggerEvent && OnSelectedItemChanged)
		OnSelectedItemChanged(NewIdx);
}

void USelectorWidget::SelectPreviousItem(const bool TriggerEvent) {
	if (Items.Num() > 0) {
		if (Items.Num() > 0) {
			if (SelectedItemIdx == 0)
				SelectedItemIdx = Items.Num();
			SetSelectedItemIndex(SelectedItemIdx - 1, TriggerEvent);
		}
	}
}

void USelectorWidget::SelectNextItem(const bool TriggerEvent) {
	if (Items.Num() > 0)
		SetSelectedItemIndex((SelectedItemIdx + 1) % Items.Num(), TriggerEvent);
}

void USelectorWidget::SetButtonsEnabled(const bool Enabled) {
	ButtonsEnabled = Enabled;

	const auto NewColor = Enabled
		? UButtonWidget::DefaultBackgroundColor
		: FLinearColor::Transparent;

	if (TBLeft) {
		if (Enabled)
			TBLeft->SetHoveredPressedCoefficients(0.8f, 0.6f);
		else
			TBLeft->SetHoveredPressedCoefficients(0.0f, 0.0f);
		TBLeft->SetBackgroundColor(NewColor);
	}
	if (TBRight) {
		if (Enabled)
			TBRight->SetHoveredPressedCoefficients(0.8f, 0.6f);
		else
			TBRight->SetHoveredPressedCoefficients(0.0f, 0.0f);
		TBRight->SetBackgroundColor(NewColor);
	}
}

void USelectorWidget::SetOnSelectedItemChangedEvent(TFunction<void(int32)> &&Event) {
	OnSelectedItemChanged = Event;
}
