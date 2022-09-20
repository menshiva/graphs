#include "ToolRemoverPanelWidget.h"
#include "ToolRemover.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UToolRemoverPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (RemoverRemoveButton)
		RemoverRemoveButton->SetOnClickEvent([&] { GetTool<UToolRemover>()->RemoveSelectedEntities(); });
	if (RemoverDeselectButton)
		RemoverDeselectButton->SetOnClickEvent([&] { GetTool<UToolRemover>()->DeselectEntities(); });
	SetButtonsEnabled(false);
}

void UToolRemoverPanelWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	static bool IsLoadingPrev = IsLoading;
	if (IsLoading != IsLoadingPrev) {
		RemoverPanelSwitcher->SetActiveWidgetIndex(IsLoading);
		SetCloseToolButtonVisible(!IsLoading);
	}
	IsLoadingPrev = IsLoading;
}

void UToolRemoverPanelWidget::SetButtonsEnabled(const bool IsEnabled) const {
	if (RemoverRemoveButton)
		RemoverRemoveButton->SetIsEnabled(IsEnabled);
	if (RemoverDeselectButton)
		RemoverDeselectButton->SetIsEnabled(IsEnabled);
}
