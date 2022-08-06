#include "ToolsPanelWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Graphs/Player/Pawn/VRPawn.h"
#include "Graphs/Player/ToolProvider/ToolProvider.h"
#include "Graphs/Player/ToolProvider/Tools/Tool.h"
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
			CloseToolButton->SetVisibility(ESlateVisibility::Collapsed);
		});
	}
}

void UToolsPanelWidget::NativeConstruct() {
	Super::NativePreConstruct();
	const auto Pawn = Cast<AVRPawn>(GetOwningPlayerPawn());
	ToolProvider = Pawn->GetToolProvider();

	// TODO: set both to 0
	size_t CurrentRow = 1, CurrentColumn = 1;
	for (const auto Tool : ToolProvider->GetTools()) {
		const auto ToolBtn = CreateWidget<UToolButtonWidget>(
			this,
			ToolButtonWidgetClass,
			FName(Tool->GetToolName().ToString() + "ToolBtn")
		);

		const auto NewSlot = ToolButtonsHolder->AddChildToUniformGrid(ToolBtn, CurrentRow, CurrentColumn++);
		NewSlot->SetHorizontalAlignment(HAlign_Center);
		NewSlot->SetVerticalAlignment(VAlign_Top);

		ToolBtn->SetToolImage(Tool->GetToolImage());
		ToolBtn->SetToolNameText(FText::FromName(Tool->GetToolName()));
		ToolBtn->SetOnClickEvent([&, Tool] {
			ToolProvider->SetActiveTool(Tool);
			ToolPanelSwitcher->SetActiveWidgetIndex(1); // TODO
			CloseToolButton->SetVisibility(ESlateVisibility::Visible);
		});

		const auto ToolPanel = Cast<UToolWidget>(CreateWidget(
			this,
			Tool->GetToolPanelClass(),
			FName(Tool->GetToolName().ToString() + "ToolPanel")
		));
		Tool->SetToolPanel(ToolPanel);
		const auto ToolPanelSlot = Cast<UWidgetSwitcherSlot>(ToolPanelSwitcher->AddChild(ToolPanel));
		ToolPanelSlot->SetHorizontalAlignment(HAlign_Fill);
		ToolPanelSlot->SetVerticalAlignment(VAlign_Fill);

		if (CurrentColumn == 2) {
			++CurrentRow;
			CurrentColumn = 0;
		}
	}
}
