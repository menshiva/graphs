#include "GraphCommands.h"
#include "EdgeCommands.h"
#include "VertexCommands.h"
#include "Graphs/GraphProvider/Entities/EdgeEntity.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/GraphProvider/Entities/VertexEntity.h"
#include "Graphs/Utils/Consts.h"

GraphCommands::Create::Create(
	EntityId *NewGraphId,
	size_t ReserveVerticesNum,
	size_t ReserveEdgesNum
) : Command([NewGraphId, ReserveVerticesNum, ReserveEdgesNum] (AGraphProvider &Provider) {
	const auto NewNode = CreateEntity<GraphEntity>(Provider);
	NewNode->VerticesIds.Reserve(ReserveVerticesNum);
	NewNode->EdgesIds.Reserve(ReserveEdgesNum);
	if (NewGraphId)
		*NewGraphId = NewNode->GetEntityId();
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

GraphCommands::Deserialize::Deserialize(
	EntityId *NewGraphId,
	rapidjson::Document &JsonDom,
	FString &ErrorMessage
) : Command([NewGraphId, &JsonDom, &ErrorMessage] (AGraphProvider &Provider) {
	*NewGraphId = ENTITY_NONE;

	// Checking validity of graph object.
	if (!JsonDom.IsObject()) {
		ErrorMessage = "Graph should be an object.";
		return;
	}
	const auto &VerticesMember = JsonDom.FindMember("vertices");
	const auto &EdgesMember = JsonDom.FindMember("edges");
	if (VerticesMember == JsonDom.MemberEnd() || !VerticesMember->value.IsArray()) {
		ErrorMessage = "Graph should have an array of objects\nnamed \"vertices\".";
		return;
	}
	if (VerticesMember->value.Size() < 1) {
		ErrorMessage = "\"vertices\" array should have\nat least 1 object.";
		return;
	}
	if (EdgesMember != JsonDom.MemberEnd() && !EdgesMember->value.IsArray()) {
		ErrorMessage = "\"edges\" field should be an array.";
		return;
	}

	// Create new graph entity.
	const auto &Vertices = VerticesMember->value.GetArray();
	Provider.ExecuteCommand(Create(
		NewGraphId,
		Vertices.Size(),
		EdgesMember != JsonDom.MemberEnd() ? EdgesMember->value.GetArray().Size() : 0
	));
	check(*NewGraphId != ENTITY_NONE);

	// Parse vertices.
	TMap<uint32_t, EntityId> VerticesIdsMappings;
	VerticesIdsMappings.Reserve(Vertices.Size());
	for (auto VertexIter = Vertices.Begin(); VertexIter != Vertices.End(); ++VertexIter) {
		EntityId NewVertexId = ENTITY_NONE;
		Provider.ExecuteCommand(VertexCommands::Deserialize(
			*NewGraphId,
			&NewVertexId,
			*VertexIter,
			ErrorMessage
		));
		if (NewVertexId != ENTITY_NONE) {
			const auto NewVertex = GetEntity<const VertexEntity>(Provider, NewVertexId);
			if (VerticesIdsMappings.Contains(NewVertex->DisplayId)) {
				ErrorMessage = "Vertex with id " + FString::FromInt(NewVertex->DisplayId) + "\nis not unique.";
				Provider.ExecuteCommand(Remove(*NewGraphId));
				*NewGraphId = ENTITY_NONE;
				return;
			}
			VerticesIdsMappings.Add(NewVertex->DisplayId, NewVertexId);
		}
		else {
			Provider.ExecuteCommand(Remove(*NewGraphId));
			*NewGraphId = ENTITY_NONE;
			return;
		}
	}

	// Parse edges if provided.
	if (EdgesMember != JsonDom.MemberEnd()) {
		const auto &Edges = EdgesMember->value.GetArray();
		for (auto EdgeIter = Edges.Begin(); EdgeIter != Edges.End(); ++EdgeIter) {
			EntityId NewEgdeId = ENTITY_NONE;
			Provider.ExecuteCommand(EdgeCommands::Deserialize(
				*NewGraphId,
				&NewEgdeId,
				*EdgeIter,
				VerticesIdsMappings,
				ErrorMessage
			));
			if (NewEgdeId == ENTITY_NONE) {
				Provider.ExecuteCommand(Remove(*NewGraphId));
				*NewGraphId = ENTITY_NONE;
				return;
			}
		}
	}
}) {}

GraphCommands::Import::Import(
	EntityId *NewGraphId,
	const FString &InputFilePath,
	FString &ErrorMessage
) : Command([NewGraphId, &InputFilePath, &ErrorMessage] (AGraphProvider &Provider) {
	*NewGraphId = ENTITY_NONE;
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (!FileManager.FileExists(*InputFilePath)) {
		ErrorMessage = "File does not exist.";
		return;
	}

	// Open selected file for reading.
	const TUniquePtr<IFileHandle> InputFileHandler(FileManager.OpenRead(*InputFilePath));
	if (!InputFileHandler.IsValid()) {
		ErrorMessage = "Failed to open file.";
		return;
	}

	// Read all bytes from file.
	TArray<uint8> InputBuffer;
	InputBuffer.AddUninitialized(InputFileHandler->Size());
	if (!InputFileHandler->Read(InputBuffer.GetData(), InputFileHandler->Size())) {
		ErrorMessage = "Failed to read from file.";
		return;
	}

	// Deserialize json using DOM style API and create a new graph.
	// https://rapidjson.org/md_doc_features.html#:~:text=DOM%20(Document%20Object%20Model)%20style%20API
	rapidjson::Document JsonDom;
	if (JsonDom.Parse(reinterpret_cast<const char*>(InputBuffer.GetData())).HasParseError()) {
		ErrorMessage = "JSON file is corrupted.";
		return;
	}
	Provider.ExecuteCommand(Deserialize(NewGraphId, JsonDom, ErrorMessage));
}) {}

GraphCommands::Serialize::Serialize(
	EntityId Id,
	rapidjson::PrettyWriter<rapidjson::StringBuffer> &Writer
) : Command([Id, &Writer] (AGraphProvider &Provider) {
	const auto Graph = GetEntity<const GraphEntity>(Provider, Id);

	Writer.StartObject();

	check(Graph->VerticesIds.Num() > 0);
	Writer.Key("vertices");
	Writer.StartArray();
	for (const auto VertexId : Graph->VerticesIds)
		Provider.ExecuteCommand(VertexCommands::Serialize(VertexId, Writer));
	Writer.EndArray();

	if (Graph->EdgesIds.Num() > 0) {
		Writer.Key("edges");
		Writer.StartArray();
		for (const auto EdgeId : Graph->EdgesIds)
			Provider.ExecuteCommand(EdgeCommands::Serialize(EdgeId, Writer));
		Writer.EndArray();
	}

	Writer.EndObject();
}) {}

GraphCommands::Export::Export(
	EntityId Id,
	const FString &OutputRootPath,
	const FString &OutputDirectory,
	bool &Result,
	FString &ResultMessage
) : Command([Id, &OutputRootPath, &OutputDirectory, &Result, &ResultMessage] (AGraphProvider &Provider) {
	auto &FileManager = FPlatformFileManager::Get().GetPlatformFile();

	// Create export directory tree if it does not exist.
	if (!FileManager.CreateDirectoryTree(*(OutputRootPath + OutputDirectory))) {
		Result = false;
		ResultMessage = "Failed to create " + OutputDirectory + " directory.";
		return;
	}

	// Generate unique filename.
	const auto DisplayDirFileBase = OutputDirectory + FileConsts::ExportFilePrefix;
	FString DisplayDirFile, OutputDirFile;
	// using do while here just to be sure that FGuid returns id of file that does not exist in output folder
	do {
		// generate new guid and remain only 8 characters for more readable file name
		auto GuidText = FGuid::NewGuid().ToString(EGuidFormats::Base36Encoded);
		check(GuidText.Len() > 8);
		GuidText.RemoveAt(8, GuidText.Len() - 8);

		DisplayDirFile = DisplayDirFileBase + GuidText + ".json";
		OutputDirFile = OutputRootPath + DisplayDirFile;
	} while (FileManager.FileExists(*OutputDirFile));

	// Create and open a new file.
	const TUniquePtr<IFileHandle> OutputFileHandler(FileManager.OpenWrite(*OutputDirFile));
	if (!OutputFileHandler.IsValid()) {
		Result = false;
		ResultMessage = "Failed to create " + DisplayDirFile + " file.";
		return;
	}

	// Serialize graph using SAX style API and write json to opened file.
	// https://rapidjson.org/md_doc_features.html#:~:text=SAX%20(Simple%20API%20for%20XML)%20style%20API
	rapidjson::StringBuffer SBuffer;
	rapidjson::PrettyWriter Writer(SBuffer);
	Writer.SetIndent('\t', 1);
	Provider.ExecuteCommand(Serialize(Id, Writer));
	if (!OutputFileHandler->Write(reinterpret_cast<const uint8*>(SBuffer.GetString()), SBuffer.GetSize())) {
		Result = false;
		ResultMessage = "Failed to write data into\n" + DisplayDirFile + " file.";
		return;
	}
	OutputFileHandler->Flush();

	Result = true;
	ResultMessage = DisplayDirFile;
}) {}
