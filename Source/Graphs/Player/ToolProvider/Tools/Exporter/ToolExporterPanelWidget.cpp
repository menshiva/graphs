#include "ToolExporterPanelWidget.h"
#include "ToolExporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/Utils/Consts.h"

void UToolExporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetOnClickEvent([&] { GetTool<UToolExporter>()->OnAttach(); });
}

void UToolExporterPanelWidget::ShowExportPanel() const {
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolExporterPanelWidget::ShowSuccessPanel(const FString &ExportedFileDir) const {
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Selected graph has been successfuly\nexported to:\n\n" + ExportedFileDir));
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor);
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::ShowErrorPanel(const FString &ErrorMessage) const {
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Error while exporting selected graph:\n\n" + ErrorMessage));
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor);
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}
