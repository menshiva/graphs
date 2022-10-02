#include "ToolsPanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Graphs/Player/ToolProvider/Tools/ToolWidget.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"
#include "Graphs/UI/Button/ToolButtonWidget.h"

UToolsPanelWidget::UToolsPanelWidget(const FObjectInitializer &ObjectInitializer) : UUserWidget(ObjectInitializer) {
	const ConstructorHelpers::FClassFinder<UUserWidget> ToolButton(TEXT("/Game/Graphs/UI/Widgets/ToolButton"));
	ToolButtonWidgetClass = ToolButton.Class;
}

void UToolsPanelWidget::NativePreConstruct() {
	Super::NativePreConstruct();
	if (CloseToolButton) {
		CloseToolButton->SetOnClickEvent([&] {
			ToolProvider->SetActiveTool(nullptr);
			ToolPanelSwitcher->SetActiveWidgetIndex(0);
			SetCloseToolButtonVisible(false);
		});
	}
}

void UToolsPanelWidget::NativeConstruct() {
	Super::NativePreConstruct();
	const auto Pawn = GetOwningPlayerPawn<AVRPawn>();
	ToolProvider = Pawn->GetToolProvider();

	for (size_t i = 0; i < ToolProvider->GetTools().Num(); ++i) {
		constexpr static size_t MaxToolsInRow = 3;

		const auto Tool = ToolProvider->GetTools()[i];
		const auto ToolBtn = CreateWidget<UToolButtonWidget>(
			this,
			ToolButtonWidgetClass,
			FName(Tool->GetToolName().ToString() + "ToolBtn")
		);

		const auto NewSlot = ToolButtonsHolder->AddChildToUniformGrid(ToolBtn, i / MaxToolsInRow, i % MaxToolsInRow);
		NewSlot->SetHorizontalAlignment(HAlign_Center);
		NewSlot->SetVerticalAlignment(VAlign_Top);

		ToolBtn->SetToolImage(Tool->GetToolImage());
		ToolBtn->SetToolNameText(FText::FromName(Tool->GetToolName()));
		ToolBtn->SetOnClickEvent([&, Tool, i] {
			ToolProvider->SetActiveTool(Tool);
			ToolPanelSwitcher->SetActiveWidgetIndex(i + 1);
			SetCloseToolButtonVisible(true);
		});

		const auto ToolPanel = Cast<UToolWidget>(CreateWidget(
			this,
			Tool->GetToolPanelClass(),
			FName(Tool->GetToolName().ToString() + "ToolPanel")
		));
		Tool->SetToolPanel(this, ToolPanel);
		const auto ToolPanelSlot = Cast<UWidgetSwitcherSlot>(ToolPanelSwitcher->AddChild(ToolPanel));
		ToolPanelSlot->SetHorizontalAlignment(HAlign_Fill);
		ToolPanelSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UToolsPanelWidget::SetCloseToolButtonVisible(const bool Visible) const {
	if (CloseToolButton)
		CloseToolButton->SetVisibility(Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
