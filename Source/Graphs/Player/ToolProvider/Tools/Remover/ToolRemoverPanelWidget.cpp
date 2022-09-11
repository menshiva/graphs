#include "ToolRemoverPanelWidget.h"
#include "ToolRemover.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UToolRemoverPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (RemoverRemoveButton)
		RemoverRemoveButton->SetOnClickEvent([&] { GetTool<UToolRemover>()->RemoveSelectedEntities(); });
	if (RemoverDeselectButton)
		RemoverDeselectButton->SetOnClickEvent([&] { GetTool<UToolRemover>()->DeselectEntities(); });
	SetButtonsEnabled(false);
}

void UToolRemoverPanelWidget::SetButtonsEnabled(const bool IsEnabled) const {
	if (RemoverRemoveButton)
		RemoverRemoveButton->SetIsEnabled(IsEnabled);
	if (RemoverDeselectButton)
		RemoverDeselectButton->SetIsEnabled(IsEnabled);
}
