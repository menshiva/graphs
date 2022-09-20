#include "ToolExporterPanelWidget.h"
#include "ToolExporter.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/Utils/Utils.h"

void UToolExporterPanelWidget::NativePreConstruct() {
	Super::NativeConstruct();
	if (ExporterConfirmButton)
		ExporterConfirmButton->SetOnClickEvent([&] { GetTool<UToolExporter>()->OnAttach(); });
}

void UToolExporterPanelWidget::NativeTick(const FGeometry &MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	static PanelType PrevPanelType = CurrentPanelType;
	if (CurrentPanelType != PrevPanelType && ExporterPanelSwitcher) {
		switch (CurrentPanelType) {
			case PanelType::SUCCESS: {
				if (ExporterText)
					ExporterText->SetText(FText::FromString("Selected graph has been successfuly\nexported to:\n\n" + Message));
				if (ExporterConfirmButton)
					ExporterConfirmButton->SetBackgroundColor(ColorConsts::GreenColor.ReinterpretAsLinear());
				ExporterPanelSwitcher->SetActiveWidgetIndex(1);
				break;
			}
			case PanelType::ERROR: {
				if (ExporterText)
					ExporterText->SetText(FText::FromString("Error while exporting selected graph:\n\n" + Message));
				if (ExporterConfirmButton)
					ExporterConfirmButton->SetBackgroundColor(ColorConsts::RedColor.ReinterpretAsLinear());
				ExporterPanelSwitcher->SetActiveWidgetIndex(1);
				break;
			}
			case PanelType::LOADING: {
				ExporterPanelSwitcher->SetActiveWidgetIndex(2);
				break;
			}
			default: {
				ExporterPanelSwitcher->SetActiveWidgetIndex(0);
			}
		}
		SetCloseToolButtonVisible(CurrentPanelType != PanelType::LOADING);
	}
	PrevPanelType = CurrentPanelType;
}
