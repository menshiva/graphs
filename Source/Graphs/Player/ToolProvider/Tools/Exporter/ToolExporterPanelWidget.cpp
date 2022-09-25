#include "ToolExporterPanelWidget.h"
#include "ToolExporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UToolExporterPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetOnClickEvent([&] { GetTool<UToolExporter>()->OnAttach(); });
}

void UToolExporterPanelWidget::ShowExportPanel() const {
	SetCloseToolButtonVisible(true);
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolExporterPanelWidget::ShowSuccessPanel(const FString &ExportPath) const {
	SetCloseToolButtonVisible(true);
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Selected graph has been successfuly\nexported to:\n\n" + ExportPath));
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::ShowErrorPanel(const FString &ErrorDesc) const {
	SetCloseToolButtonVisible(true);
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Error while exporting selected graph:\n\n" + ErrorDesc));
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::ShowLoadingPanel() const {
	SetCloseToolButtonVisible(false);
	if (ExporterPanelSwitcher)
		ExporterPanelSwitcher->SetActiveWidgetIndex(2);
}
