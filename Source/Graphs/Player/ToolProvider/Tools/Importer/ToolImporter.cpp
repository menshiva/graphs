#include "ToolImporter.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/EntityStorage/Commands/GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolImporter::ImportGraphFromFile"), STAT_UToolImporter_ImportGraphFromFile, STATGROUP_GRAPHS_PERF);

UToolImporter::UToolImporter() : UTool(
	"Import",
	TEXT("/Game/Graphs/UI/Icons/Import"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolImporterPanel")
) {}

void UToolImporter::OnAttach() {
	Super::OnAttach();
	GetToolPanel<UToolImporterPanelWidget>()->ShowImportList();
}

void UToolImporter::OnDetach() {
	Super::OnDetach();
	DeselectImportedGraph();
}

void UToolImporter::ImportFromFile(const FString &FilePath) {
	const auto GraphRenderers = GetGraphsRenderers();
	const auto ImporterToolPanel = GetToolPanel<UToolImporterPanelWidget>();
	ImporterToolPanel->ShowLoadingPanel();

	AsyncTask(
		ENamedThreads::AnyBackgroundHiPriTask,
		[&, GraphRenderers, ImporterToolPanel] {
			auto ErrorMessage = ImportFromFileImpl(FilePath);
			Utils::DoOnGameThread([&, GraphRenderers, ImporterToolPanel, ErrorMessage(MoveTemp(ErrorMessage))] {
				if (ImportedGraphId != EntityId::NONE()) {
					check(ErrorMessage.IsEmpty());
					ImporterToolPanel->ShowSuccessPanel();
					GraphCommands::Mutable::SetOverrideColor(ImportedGraphId, ColorConsts::GreenColor);
					GraphRenderers->ConstructGraphChunks(ImportedGraphId);
				}
				else {
					check(!ErrorMessage.IsEmpty());
					ImporterToolPanel->ShowErrorPanel("Error while importing graph:\n\n" + ErrorMessage);
				}
			});
		}
	);
}

void UToolImporter::DeselectImportedGraph() {
	if (ImportedGraphId != EntityId::NONE()) {
		GraphCommands::Mutable::SetOverrideColor(ImportedGraphId, ColorConsts::OverrideColorNone);
		GetGraphsRenderers()->MarkGraphDirty(
			ImportedGraphId,
			true, false,
			true, false
		);
		GetGraphsRenderers()->RedrawGraphChunksIfDirty(ImportedGraphId);
		ImportedGraphId = EntityId::NONE();
	}
}

FString UToolImporter::ImportFromFileImpl(const FString &FilePath) {
	SCOPE_CYCLE_COUNTER(STAT_UToolImporter_ImportGraphFromFile);

	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	check(FilePath.EndsWith(".json"));

	if (!FileManager.FileExists(*FilePath))
		return "File does not exist.";

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, &FileManager, *FilePath))
		return "Failed to read from file.";

	FString ErrorMessage;
	ImportedGraphId = GraphCommands::Mutable::Deserialize(JsonStr, ErrorMessage);
	return ErrorMessage;
}
