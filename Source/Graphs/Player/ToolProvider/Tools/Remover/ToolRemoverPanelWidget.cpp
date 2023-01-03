#include "ToolRemoverPanelWidget.h"
#include "ToolRemover.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UToolRemoverPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	SetButtonsEnabled(false);
}

void UToolRemoverPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	const auto RemoverTool = GetTool<UToolRemover>();

	RemoverRemoveButton->SetOnClickEvent([&, RemoverTool] {
		RemoverTool->RemoveSelectedEntities();
	});

	RemoverDeselectButton->SetOnClickEvent([&, RemoverTool] {
		RemoverTool->DeselectEntities();
	});
}

void UToolRemoverPanelWidget::SetLoadingStatus(const bool Loading) const {
	RemoverPanelSwitcher->SetActiveWidgetIndex(Loading);
	SetCloseToolButtonVisible(!Loading);
}

void UToolRemoverPanelWidget::SetButtonsEnabled(const bool IsEnabled) const {
	if (RemoverRemoveButton)
		RemoverRemoveButton->SetIsEnabled(IsEnabled);
	if (RemoverDeselectButton)
		RemoverDeselectButton->SetIsEnabled(IsEnabled);
}
