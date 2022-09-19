#include "ToolExporter.h"
#include "ToolExporterPanelWidget.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"

DECLARE_CYCLE_STAT(TEXT("UToolExporter::OnRightTriggerAction"), STAT_UToolExporter_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("UToolExporter::ExportGraph"), STAT_UToolExporter_ExportGraph, STATGROUP_GRAPHS_PERF);

UToolExporter::UToolExporter() : UTool(
	"Export",
	TEXT("/Game/Graphs/UI/Icons/Export"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolExporterPanel")
) {
	SetSupportedEntities({GRAPH});
}

void UToolExporter::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
	GetToolPanel<UToolExporterPanelWidget>()->SetPanelType(UToolExporterPanelWidget::PanelType::NONE);
}

void UToolExporter::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

bool UToolExporter::OnRightTriggerAction(const bool IsPressed) {
	SCOPE_CYCLE_COUNTER(STAT_UToolExporter_OnRightTriggerAction);

	if (IsPressed) {
		const auto ExporterToolPanel = GetToolPanel<UToolExporterPanelWidget>();
		auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();

		const auto &GameDirPath = FPaths::LaunchDir();
		const auto &ExportDirName = FileConsts::ExportDirName;
		const auto ExportDirPath = GameDirPath + ExportDirName;

		if (!FileManager.CreateDirectoryTree(*ExportDirPath)) {
			ExporterToolPanel->SetMessage("Failed to create export directory.");
			ExporterToolPanel->SetPanelType(UToolExporterPanelWidget::PanelType::ERROR);
			return false;
		}

		const auto NewFileName = GenerateJsonFileNameInDirectory(
			FileManager,
			ExportDirPath,
			FileConsts::ExportFilePrefix
		);

		auto OutputPath = ExportDirPath + NewFileName;
		auto OutputDisplayPath = ExportDirName + NewFileName;
		const auto GraphId = GetHitEntityId();

		GetVrRightController()->SetLaserActive(false);
		ExporterToolPanel->SetPanelType(UToolExporterPanelWidget::PanelType::LOADING);

		AsyncTask(
			ENamedThreads::AnyBackgroundHiPriTask,
			[GraphId, OutputPath(MoveTemp(OutputPath)), OutputDisplayPath(MoveTemp(OutputDisplayPath)), ExporterToolPanel] {
				FString ErrorMessage;
				if (ExportGraph(GraphId, FPlatformFileManager::Get().GetPlatformFile(), OutputPath, ErrorMessage)) {
					ExporterToolPanel->SetMessage(OutputDisplayPath);
					ExporterToolPanel->SetPanelType(UToolExporterPanelWidget::PanelType::SUCCESS);
				}
				else {
					ExporterToolPanel->SetMessage(ErrorMessage);
					ExporterToolPanel->SetPanelType(UToolExporterPanelWidget::PanelType::ERROR);
				}
			}
		);

		return true;
	}

	return Super::OnRightTriggerAction(IsPressed);
}

FString UToolExporter::GenerateJsonFileNameInDirectory(
	IPlatformFile &FileManager,
	const FString &DirPath,
	const FString &FilePrefix
) {
	check(FileManager.DirectoryExists(*DirPath));

	FString NewFileName;

	// using do while here just to be sure that FGuid returns id that will be used in the name of file
	// which does not exist in given directory
	do {
		// generate new guid and remain only 8 characters for more readable file name
		auto GuidText = FGuid::NewGuid().ToString(EGuidFormats::Base36Encoded);
		check(GuidText.Len() > 8);
		GuidText.RemoveAt(8, GuidText.Len() - 8);
		NewFileName = FilePrefix + GuidText + ".json";
	} while (FileManager.FileExists(*(DirPath + NewFileName)));

	return NewFileName;
}

bool UToolExporter::ExportGraph(
	const EntityId GraphId,
	IPlatformFile &FileManager,
	const FString &OutputPath,
	FString &ErrorMessage
) {
	SCOPE_CYCLE_COUNTER(STAT_UToolExporter_ExportGraph);

	TUniquePtr<IFileHandle> OutputFileHandler(FileManager.OpenWrite(*OutputPath, false, false));
	if (!OutputFileHandler.IsValid()) {
		ErrorMessage = "Failed to create a new file.";
		return false;
	}

	rapidjson::StringBuffer SBuffer;
	rapidjson::PrettyWriter Writer(SBuffer);
	Writer.SetIndent('\t', 1);

	GraphCommands::ConstFuncs::Serialize(GraphId, Writer);

	if (!OutputFileHandler->Write(reinterpret_cast<const uint8*>(SBuffer.GetString()), SBuffer.GetSize())) {
		OutputFileHandler.Reset();
		FileManager.DeleteFile(*OutputPath);
		ErrorMessage = "Failed to write data to a new file.";
		return false;
	}
	OutputFileHandler->Flush();

	return true;
}
