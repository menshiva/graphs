#include "ToolEditorPanelWidget.h"
#include "ToolEditor.h"
#include "Components/EditableText.h"
#include "Components/GridPanel.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Graphs/Player/Menu/MenuWidget.h"
#include "Graphs/UI/Button/ImageButtonWidget.h"
#include "Graphs/UI/Button/TextButtonWidget.h"
#include "Graphs/UI/Checkbox/CheckboxWidget.h"

void UToolEditorPanelWidget::NativeConstruct() {
	Super::NativeConstruct();

	ParentMenu = GetOwningPlayerPawn<AVRPawn>()->GetMenuWidget();
	const auto EditorTool = GetTool<UToolEditor>();

	ColorfulCheckbox->SetOnClickEvent([&, EditorTool] {
		SetDataChanged();
		if (EditorTool->InvertGraphColorful()) {
			RandomizeVerticesColorsButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			ColorHolderPanelSwitcher->SetActiveWidgetIndex(1);
			ColorHolder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
			ColorHolder->SetVisibility(ESlateVisibility::Collapsed);
	});

	for (const auto ColorButton : {Pink, Purple, Cyan, Teal, Yellow, Orange, Brown}) {
		ColorButton->SetOnClickEvent([&, EditorTool, Color = ColorButton->GetImageColor()] {
			SetEntityColor(EditorTool, Color);
		});
	}

	RandomColor->SetOnClickEvent([&, EditorTool] {
		SetEntityColor(EditorTool, FLinearColor::MakeRandomColor());
	});

	RandomizeVerticesColorsButton->SetOnClickEvent([&, EditorTool] {
		SetDataChanged();
		EditorTool->RandomizeVerticesColors();
	});

	VertexDataInput->OnTextChanged.AddDynamic(this, &UToolEditorPanelWidget::OnEditTextChanged);
	VertexDataInput->OnTextCommitted.AddDynamic(this, &UToolEditorPanelWidget::OnEditTextFocusLost);

	EditorSaveButton->SetOnClickEvent([EditorTool] {
		EditorTool->SelectEntity(EntityId::NONE());
	});
	EditorCancelButton->SetOnClickEvent([&, EditorTool] {
		if (IsDataChanged)
			EditorTool->RestoreCache();
		EditorTool->SelectEntity(EntityId::NONE());
	});
}

void UToolEditorPanelWidget::NativeTick(const FGeometry &MyGeometry, const float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
	// VertexDataInput editable text has no OnFocus event, so we check focus on tick event
	if (VertexValueHolder->IsVisible() && !ParentMenu->IsKeyboardVisible() && VertexDataInput->HasAnyUserFocus()) {
		CloseKeyboardBtn->SetIsEnabled(true);
		ParentMenu->SetKeyboardVisibility(true);
	}
}

void UToolEditorPanelWidget::Update() {
	const auto SelectedId = GetTool<UToolEditor>()->GetSelectedEntityId();

	if (SelectedId == EntityId::NONE()) {
		EditorPanelSwitcher->SetActiveWidgetIndex(0);
		IsDataChanged = false;
		EditorSaveButton->SetIsEnabled(false);
	}
	else {
		if (ES::IsValid<VertexEntity>(SelectedId)) {
			const auto &Vertex = ES::GetEntity<VertexEntity>(SelectedId);
			const auto &Graph = ES::GetEntity<GraphEntity>(Vertex.GraphId);

			SelectedEntityTitle->SetText(FText::FromString("Vertex #" + FString::FromInt(Vertex.CustomId)));

			ColorfulCheckbox->SetVisibility(ESlateVisibility::Collapsed);

			RandomizeVerticesColorsButton->SetVisibility(ESlateVisibility::Collapsed);
			ColorHolderPanelSwitcher->SetActiveWidgetIndex(Graph.Colorful);
			ColorHolder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			VertexDataInput->SetText(FText::FromString(FString::FromInt(Vertex.Value)));
			VertexValueHolder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else if (ES::IsValid<GraphEntity>(SelectedId)) {
			const auto &Graph = ES::GetEntity<GraphEntity>(SelectedId);

			SelectedEntityTitle->SetText(FText::FromString("Graph"));

			ColorfulCheckbox->SetTicked(Graph.Colorful);
			ColorfulCheckbox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

			if (Graph.Colorful) {
				RandomizeVerticesColorsButton->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
				ColorHolderPanelSwitcher->SetActiveWidgetIndex(1);
				ColorHolder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			}
			else
				ColorHolder->SetVisibility(ESlateVisibility::Collapsed);

			VertexValueHolder->SetVisibility(ESlateVisibility::Collapsed);
		}
		else {
			check(false);
		}

		EditorPanelSwitcher->SetActiveWidgetIndex(1);
	}
}

void UToolEditorPanelWidget::OnEditTextChanged(const FText &Text) {
	SetDataChanged();
	GetTool<UToolEditor>()->SetVertexValue(FCString::Atoi(*Text.ToString()));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UToolEditorPanelWidget::OnEditTextFocusLost(const FText&, ETextCommit::Type) {
	ParentMenu->SetKeyboardVisibility(false);
	CloseKeyboardBtn->SetIsEnabled(false);
}

void UToolEditorPanelWidget::SetEntityColor(const UToolEditor *EditorTool, const FLinearColor &Color) {
	SetDataChanged();
	if (ES::IsValid<VertexEntity>(EditorTool->GetSelectedEntityId())) {
		EditorTool->SetVertexColor(Color.ToFColor(false));
	}
	else {
		check(ES::IsValid<GraphEntity>(EditorTool->GetSelectedEntityId()));
		EditorTool->SetGraphColor(Color.ToFColor(false));
	}
}

void UToolEditorPanelWidget::SetDataChanged() {
	IsDataChanged = true;
	EditorSaveButton->SetIsEnabled(true);
}
