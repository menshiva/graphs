#include "ToolExporterPanelWidget.h"
#include "ToolExporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"

void UToolExporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	ExporterConfirmButton->SetOnClickEvent([ExporterTool = GetTool<UToolExporter>()] {
		ExporterTool->OnAttach();
	});
}

void UToolExporterPanelWidget::ShowExportPanel() const {
	SetCloseToolButtonVisible(true);
	ExporterPanelSwitcher->SetActiveWidgetIndex(0);
}

void UToolExporterPanelWidget::ShowSuccessPanel(const FString &ExportPath) const {
	SetCloseToolButtonVisible(true);
	ExporterText->SetText(FText::FromString("Selected graph has been successfuly\nexported to:\n\n" + ExportPath));
	ExporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
	ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::ShowErrorPanel(const FStringView &ErrorDesc) const {
	SetCloseToolButtonVisible(true);
	ExporterText->SetText(FText::FromString(FString("Error while exporting selected graph:\n\n") + ErrorDesc.GetData()));
	ExporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
	ExporterPanelSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::ShowLoadingPanel() const {
	SetCloseToolButtonVisible(false);
	ExporterPanelSwitcher->SetActiveWidgetIndex(2);
}
