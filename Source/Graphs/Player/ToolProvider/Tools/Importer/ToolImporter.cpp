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
	RefreshFileList();
	GetToolPanel<UToolImporterPanelWidget>()->ShowImportList();
}

void UToolImporter::OnImportClick(const FString &FilePath) const {
	const auto GraphRenderers = GetGraphsRenderers();
	const auto ImporterToolPanel = GetToolPanel<UToolImporterPanelWidget>();
	ImporterToolPanel->ShowLoadingPanel();

	AsyncTask(
		ENamedThreads::AnyBackgroundHiPriTask,
		[GraphRenderers, &FilePath, ImporterToolPanel] {
			FString ErrorMessage;
			const auto GraphId = ImportGraphFromFile(FilePath, ErrorMessage);

			Utils::DoOnGameThread([=, ErrorMessage(MoveTemp(ErrorMessage))] {
				if (GraphId != EntityId::NONE()) {
					ImporterToolPanel->ShowSuccessPanel();
					GraphRenderers->ConstructGraphChunks(GraphId);
				}
				else {
					ImporterToolPanel->ShowErrorPanel("Error while importing graph:\n\n" + ErrorMessage);
				}
			});
		}
	);
}

void UToolImporter::RefreshFileList() const {
	const auto ImporterToolPanel = GetToolPanel<UToolImporterPanelWidget>();
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto ExportDirPath = FPaths::LaunchDir() + FileConsts::ExportDirName;

	if (!FileManager.CreateDirectoryTree(*ExportDirPath)) {
		ImporterToolPanel->ShowErrorPanel("Failed to create export directory.");
		return;
	}

	TArray<FString> ImportFiles;
	FileManager.FindFiles(ImportFiles, *ExportDirPath, TEXT(".json"));
	ImporterToolPanel->SetInputFiles(MoveTemp(ImportFiles));
}

EntityId UToolImporter::ImportGraphFromFile(const FString &FilePath, FString &ErrorMessage) {
	SCOPE_CYCLE_COUNTER(STAT_UToolImporter_ImportGraphFromFile);

	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	check(FilePath.EndsWith(".json"));

	if (!FileManager.FileExists(*FilePath)) {
		ErrorMessage = "File does not exist.";
		return EntityId::NONE();
	}

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, &FileManager, *FilePath)) {
		ErrorMessage = "Failed to read from file.";
		return EntityId::NONE();
	}

	return GraphCommands::Mutable::Deserialize(JsonStr, ErrorMessage);
}
