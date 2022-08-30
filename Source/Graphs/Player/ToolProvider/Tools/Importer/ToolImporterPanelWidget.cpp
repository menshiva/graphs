#include "ToolImporterPanelWidget.h"
#include "ToolImporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/Utils/Consts.h"

void UToolImporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ImporterImportButton)
		ImporterImportButton->SetOnClickEvent([&] { GetTool<UToolImporter>()->ImportGraph(); });
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetOnClickEvent([&] { ShowImportPanel(); });
}

void UToolImporterPanelWidget::ShowImportPanel() const {
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolImporterPanelWidget::ShowSuccessPanel() const {
	if (ImporterText)
		ImporterText->SetText(FText::FromString("Graph has been successfuly\nimported."));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolImporterPanelWidget::ShowErrorPanel(const FString &ErrorMessage) const {
	if (ImporterText)
		ImporterText->SetText(FText::FromString("Error while importing graph:\n\n" + ErrorMessage));
	if (ImporterConfirmButton)
		ImporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor);
	if (ImporterPanelSwitcher)
		ImporterPanelSwitcher->SetActiveWidgetIndex(1);
}
