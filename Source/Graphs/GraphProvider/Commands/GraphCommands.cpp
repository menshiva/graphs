#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/EdgeEntity.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"
#include "ThirdParty/nlohmann/json.hpp"

GraphCommands::Create::Create(EntityId *NewId) : Command([NewId] (AGraphProvider &Provider) {
	const auto NewNode = CreateEntity<GraphEntity>(Provider);
	if (NewId)
		*NewId = NewNode->GetEntityId();
}) {}

GraphCommands::Remove::Remove(EntityId Id) : Command([Id] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		RemoveEntity(Provider, VertexId);
	for (const auto EdgeId : Graph->EdgesIds)
		RemoveEntity(Provider, EdgeId);
	RemoveEntity(Provider, Id);
}) {}

GraphCommands::SetColor::SetColor(
	EntityId Id,
	const FLinearColor &Color
) : Command([Id, &Color] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetColor(VertexId, Color));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetColor(EdgeId, Color));
}) {}

GraphCommands::SetSelectionType::SetSelectionType(
	EntityId Id,
	SelectionType NewType
) : Command([Id, NewType] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::SetSelectionType(VertexId, NewType));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::SetSelectionType(EdgeId, NewType));
}) {}

GraphCommands::Move::Move(
	EntityId Id,
	const FVector &Delta
) : Command([Id, &Delta] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::Move(VertexId, Delta, false));
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::Move(EdgeId, Delta, false));
}) {}

GraphCommands::ComputeCenterPosition::ComputeCenterPosition(
	EntityId Id,
	FVector &Center
) : Command([Id, &Center] (const AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	check(Graph->VerticesIds.Num() > 0);
	Center = FVector::ZeroVector;
	for (const auto VertexId : Graph->VerticesIds)
		Center += GetEntity<const VertexEntity>(Provider, VertexId)->Actor->GetActorLocation();
	Center /= Graph->VerticesIds.Num();
}) {}

GraphCommands::Rotate::Rotate(
	EntityId Id,
	const FVector &Center,
	const float Angle
) : Command([Id, &Center, Angle] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);
	for (const auto VertexId : Graph->VerticesIds) {
		const auto Vertex = GetEntity<const VertexEntity>(Provider, VertexId);
		const auto VertexPos = Vertex->Actor->GetActorLocation();
		const auto PosDirection = VertexPos - Center;
		const auto RotatedPos = PosDirection.RotateAngleAxis(Angle, FVector::DownVector) + Center;
		Provider.ExecuteCommand(VertexCommands::Move(VertexId, RotatedPos - VertexPos, false));
	}
	for (const auto EdgeId : Graph->EdgesIds)
		Provider.ExecuteCommand(EdgeCommands::UpdateTransform(EdgeId));
}) {}

GraphCommands::Export::Export(
	EntityId Id,
	bool &Result,
	FString &ResultMessage
) : Command([Id, &Result, &ResultMessage] (const AGraphProvider &Provider) {
	IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();

	// Create export directory if it does not exist.
	const auto OutputDir = FPaths::LaunchDir() + FileConsts::ImportExportDir;
	if (!FileManager.CreateDirectory(*OutputDir)) {
		auto OutputFloderName = FileConsts::ImportExportDir;
		OutputFloderName.RemoveAt(OutputFloderName.Len() - 1);

		Result = false;
		ResultMessage = "Failed to create a new folder:\n" + OutputFloderName;
		return;
	}

	// Generate unique filename for exporting graph.
	const auto DisplayDirFileBase = FileConsts::ImportExportDir + FileConsts::ExportFilePrefix;
	FString DisplayDirFile, OutputDirFile;
	// using do while here just to be sure that FGuid returns id of file that does not exist in output folder
	do {
		// generate new guid and remain only 8 characters for more readable file name
		auto GuidText = FGuid::NewGuid().ToString(EGuidFormats::Base36Encoded);
		GuidText.RemoveAt(8, GuidText.Len() - 8);

		DisplayDirFile = DisplayDirFileBase + GuidText + ".json";
		OutputDirFile = FPaths::LaunchDir() + DisplayDirFile;
	} while (FileManager.FileExists(*OutputDirFile));

	// Create and open a new file.
	const auto OutputFileHandler = FileManager.OpenWrite(*OutputDirFile);
	if (!OutputFileHandler) {
		Result = false;
		ResultMessage = "Failed to create a new file:\n" + DisplayDirFile;
		return;
	}

	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	// Graph's vertices serialization.
	nlohmann::json VerticesJson;
	for (const auto VertexId : Graph->VerticesIds) {
		const auto Vertex = GetEntity<const VertexEntity>(Provider, VertexId);
		const auto VertexPos = Vertex->Actor->GetActorLocation();

		VerticesJson.push_back(nlohmann::json::object({
			{"id", Vertex->DisplayId},
			{"position", nlohmann::json::object({
				{"x", VertexPos.X},
				{"y", VertexPos.Y},
				{"z", VertexPos.Z}
			})},
		}));
	}

	// Graph's edges serialization.
	nlohmann::json EdgesJson;
	for (const auto EdgeId : Graph->EdgesIds) {
		const auto Edge = GetEntity<const EdgeEntity>(Provider, EdgeId);
		const auto FromVertex = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[0]);
		const auto ToVertex = GetEntity<const VertexEntity>(Provider, Edge->VerticesIds[1]);

		EdgesJson.push_back(nlohmann::json::object({
			{"vertices", nlohmann::json::object({
				{"from_id", FromVertex->DisplayId},
				{"to_id", ToVertex->DisplayId}
			})},
		}));
	}

	// Graph serialization.
	const nlohmann::json GraphJson = nlohmann::json::object({
		{"vertices", VerticesJson},
		{"edges", EdgesJson}
	});
	const auto GraphJsonStr = GraphJson.dump(1, '\t');
	OutputFileHandler->Write(reinterpret_cast<const uint8*>(GraphJsonStr.c_str()), GraphJsonStr.size());

	OutputFileHandler->Flush();
	delete OutputFileHandler;

	Result = true;
	ResultMessage = DisplayDirFile;
}) {}
