#include "ToolImporter.h"
#include "ToolImporterPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolImporter::ImportGraphFromFile"), STAT_UToolImporter_ImportGraphFromFile, STATGROUP_GRAPHS_PERF);

UToolImporter::UToolImporter() : UTool(
	"Import",
	TEXT("/Game/Graphs/UI/Icons/Import"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolImporterPanel")
) {}

void UToolImporter::OnAttach() {
	Super::OnAttach();
	RefreshFileList();
}

void UToolImporter::RefreshFileList() const {
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();
	const auto ExportDirPath = FPaths::LaunchDir() + FileConsts::ExportDirName;

	if (!FileManager.CreateDirectoryTree(*ExportDirPath)) {
		GetToolPanel<UToolImporterPanelWidget>()->ShowErrorPanel("Failed to create export directory.");
		return;
	}

	TArray<FString> ImportFiles;
	FileManager.FindFiles(ImportFiles, *ExportDirPath, TEXT(".json"));

	const auto ImporterUI = GetToolPanel<UToolImporterPanelWidget>();
	ImporterUI->SetInputFiles(ImportFiles);
	ImporterUI->ShowImportPanel();
}

bool UToolImporter::ImportGraphFromFile(const FString &FilePath, FString &ErrorMessage) const {
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

	return GetGraphRenderer()->ExecuteCommand(GraphCommands::Deserialize(JsonStr, ErrorMessage), true);
}
