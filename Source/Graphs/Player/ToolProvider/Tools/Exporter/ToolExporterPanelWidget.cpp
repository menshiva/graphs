#include "ToolExporterPanelWidget.h"
#include "ToolExporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/Utils/Consts.h"

void UToolExporterPanelWidget::NativeConstruct() {
	Super::NativeConstruct();
	if (ExporterButton)
		ExporterButton->SetOnClickEvent([&] { GetTool<UToolExporter>()->OnAttach(); });
}

void UToolExporterPanelWidget::SetTutorialStatus() const {
	if (ExporterStatusSwitcher)
		ExporterStatusSwitcher->SetActiveWidgetIndex(0);
}

void UToolExporterPanelWidget::SetSuccessStatus(const FString &ExportedFileDir) const {
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Selected graph has been\nsuccessfuly exported to:\n\n" + ExportedFileDir));
	if (ExporterButton)
		ExporterButton->SetBackgroundColor(ColorConsts::GreenColor);
	if (ExporterStatusSwitcher)
		ExporterStatusSwitcher->SetActiveWidgetIndex(1);
}

void UToolExporterPanelWidget::SetErrorStatus(const FString &ErrorDescription) const {
	if (ExporterText)
		ExporterText->SetText(FText::FromString("Error while exporting selected graph:\n\n" + ErrorDescription));
	if (ExporterButton)
		ExporterButton->SetBackgroundColor(ColorConsts::RedColor);
	if (ExporterStatusSwitcher)
		ExporterStatusSwitcher->SetActiveWidgetIndex(1);
}
