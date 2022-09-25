#include "ToolImporter.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/GraphsRenderers/Commands/GraphCommands.h"

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
			GraphImportData ImportData;
			FString ErrorMessage;

			if (!ImportGraphFromFile(FilePath, ImportData, ErrorMessage)) {
				Utils::DoOnGameThread([ImporterToolPanel, ErrorMessage(MoveTemp(ErrorMessage))] {
					ImporterToolPanel->ShowErrorPanel("Error while importing graph:\n\n" + ErrorMessage);
				});
				return;
			}

			Utils::DoOnGameThread([GraphRenderers, ImportData(MoveTemp(ImportData)), ImporterToolPanel] () mutable {
				EntityId NewGraphId = EntityId::NONE();
				GraphRenderers->ExecuteCommand(GraphCommands::Create(&NewGraphId, ImportData.Colorful));
				check(NewGraphId != EntityId::NONE());
				GraphRenderers->AddGraphRenderer(NewGraphId);

				AsyncTask(
					ENamedThreads::AnyBackgroundHiPriTask,
					[=, ImportData(MoveTemp(ImportData))] {
						GraphRenderers->ExecuteCommand(GraphCommands::FillFromImportData(NewGraphId, ImportData));

						Utils::DoOnGameThread([GraphRenderers, ImporterToolPanel] {
							GraphRenderers->RedrawIfDirty();
							ImporterToolPanel->ShowSuccessPanel();
						});
					}
				);
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

	ImporterToolPanel->SetInputFiles(ImportFiles);
}

bool UToolImporter::ImportGraphFromFile(
	const FString &FilePath,
	GraphImportData &ImportData,
	FString &ErrorMessage
) {
	SCOPE_CYCLE_COUNTER(STAT_UToolImporter_ImportGraphFromFile);

	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	check(FilePath.EndsWith(".json"));

	if (!FileManager.FileExists(*FilePath)) {
		ErrorMessage = "File does not exist.";
		return false;
	}

	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, &FileManager, *FilePath)) {
		ErrorMessage = "Failed to read from file.";
		return false;
	}

	return GraphCommands::Consts::Deserialize(JsonStr, ImportData, ErrorMessage);
}
