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
	GetVrRightController()->SetLaserActive(false);
}

void UToolImporter::OnDetach() {
	Super::OnDetach();
	DeselectImportedGraph();
	GetVrRightController()->SetLaserActive(true);
}

bool UToolImporter::GetExportFolderContents(TArray<FString> &OutFilePaths) {
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto ExportDirPath = FPaths::LaunchDir() + FileConsts::ExportDirName;
	if (!FileManager.CreateDirectoryTree(*ExportDirPath))
		return false;
	FileManager.FindFiles(OutFilePaths, *ExportDirPath, TEXT(".json"));
	return true;
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
				if (ES::IsValid<GraphEntity>(ImportedGraphId)) {
					check(ErrorMessage.IsEmpty());
					ImporterToolPanel->ShowSuccessPanel();
					GraphCommands::Mutable::SetOverrideColor(ImportedGraphId, ColorConsts::GreenColor);
					GraphRenderers->ConstructGraphChunks(ImportedGraphId);
				}
				else {
					check(!ErrorMessage.IsEmpty());
					ImporterToolPanel->ShowErrorPanel("Error while importing the graph:\n\n" + ErrorMessage);
				}
			});
		}
	);
}

void UToolImporter::DeselectImportedGraph() {
	if (ES::IsValid<GraphEntity>(ImportedGraphId)) {
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
		return "File does not exist. Please refresh the list.";

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, &FileManager, *FilePath))
		return "Failed to read file contents.";

	FString ErrorMessage;
	ImportedGraphId = GraphCommands::Mutable::Deserialize(JsonStr, ErrorMessage);

	return ErrorMessage;
}
