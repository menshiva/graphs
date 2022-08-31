#include "ToolExporter.h"
#include "ToolExporterPanelWidget.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/Utils/Consts.h"

UToolExporter::UToolExporter() : UTool(
	"Export",
	TEXT("/Game/Graphs/UI/Icons/Export"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolExporterPanel")
) {
	SetSupportedEntityTypes({EntityType::GRAPH});
}

void UToolExporter::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
	GetToolPanel<UToolExporterPanelWidget>()->ShowExportPanel();
}

void UToolExporter::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

bool UToolExporter::OnRightTriggerAction(const bool IsPressed) {
	if (IsPressed && GetHitEntityId() != ENTITY_NONE) {
		check(GetGraphProvider()->GetEntityType(GetHitEntityId()) == EntityType::GRAPH);

		bool IsOk;
		FString ResultMsg;
		GetGraphProvider()->ExecuteCommand(GraphCommands::Export(
			GetHitEntityId(),
			FPaths::LaunchDir(),
			FileConsts::ExportDir,
			IsOk,
			ResultMsg
		));

		if (IsOk)
			GetToolPanel<UToolExporterPanelWidget>()->ShowSuccessPanel(ResultMsg);
		else
			GetToolPanel<UToolExporterPanelWidget>()->ShowErrorPanel(ResultMsg);

		GetVrRightController()->SetLaserActive(false);
		return true;
	}
	return Super::OnRightTriggerAction(IsPressed);
}
