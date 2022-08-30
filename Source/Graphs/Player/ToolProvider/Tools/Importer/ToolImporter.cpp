#include "ToolImporter.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"

UToolImporter::UToolImporter() : UTool(
	"Import",
	TEXT("/Game/Graphs/UI/Icons/Import"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolImporterPanel")
) {}

void UToolImporter::OnAttach() {
	Super::OnAttach();
	GetToolPanel<UToolImporterPanelWidget>()->ShowImportPanel();
}

void UToolImporter::ImportGraph() const {
	GraphCommands::Import::ResultType Result;
	FString ErrorMsg;
	GetGraphProvider()->ExecuteCommand(GraphCommands::Import(nullptr, Result, ErrorMsg));

	if (Result == GraphCommands::Import::ResultType::SUCCESS)
		GetToolPanel<UToolImporterPanelWidget>()->ShowSuccessPanel();
	else if (Result == GraphCommands::Import::ResultType::ERROR)
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel(ErrorMsg);
}
