﻿#include "GraphsRenderers.h"

AGraphsRenderers::AGraphsRenderers() {
	PrimaryActorTick.bCanEverTick = false;
}

EntityId AGraphsRenderers::GetEntityIdFromHitResult(const FHitResult &HitResult) const {
	if (HitResult.bBlockingHit)
		if (const auto Chunk = Cast<AGraphChunkRenderer>(HitResult.GetActor()))
			return Chunk->GetEntityIdFromHitResult(HitResult);
	return EntityId::NONE();
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::ConstructGraphChunks"), STAT_AGraphsRenderers_ConstructGraphChunks, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::ConstructGraphChunks(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_ConstructGraphChunks);

	check(IsInGameThread());
	check(!GraphsChunks.IsValidIndex(GraphId.GetIndex()));

	// compute new chunks num
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);
	const size_t NewChunksNum = ceilf(
		static_cast<float>(FMath::Max(Graph.Vertices.Num(), Graph.Edges.Num()))
			/ static_cast<float>(AGraphChunkRenderer::ChunkSize)
	);
	check(NewChunksNum > 0);

	// create chunks
	AllChunks.Reserve(AllChunks.Num() + NewChunksNum);
	const auto NewGraphChunksIdx = GraphsChunks.Emplace();
	check(NewGraphChunksIdx == GraphId.GetIndex());
	auto &NewGraphChunks = GraphsChunks[NewGraphChunksIdx];
	NewGraphChunks.Reserve(NewChunksNum);
	for (size_t i = 0; i < NewChunksNum; ++i) {
		FActorSpawnParameters Params;
		Params.Owner = this;
		const auto NewChunk = GetWorld()->SpawnActor<AGraphChunkRenderer>(Params);
		bool AlreadyInSet = false;
		AllChunks.Add(NewChunk, &AlreadyInSet);
		check(!AlreadyInSet);
		NewGraphChunks.Add(NewChunk, &AlreadyInSet);
		check(!AlreadyInSet);
	}

	// fill new chunks with vertices and edges from graph
	VerticesChunksLookup.Reserve(VerticesChunksLookup.Num() + Graph.Vertices.Num());
	auto ChunkIter = NewGraphChunks.CreateConstIterator();
	for (const auto VertexId : Graph.Vertices) {
		auto CurrentChunk = *ChunkIter;
		if (!CurrentChunk->AddVertex(VertexId)) {
			CurrentChunk = *(++ChunkIter);
			const bool Result = CurrentChunk->AddVertex(VertexId);
			check(Result);
		}

		// use Insert if check below fails
		const auto LookupIdx = VerticesChunksLookup.Add(CurrentChunk);
		check(VertexId.GetIndex() == LookupIdx);
	}
	EdgesChunksLookup.Reserve(EdgesChunksLookup.Num() + Graph.Edges.Num());
	auto OtherChunkIter = NewGraphChunks.CreateConstIterator();
	for (const auto EdgeId : Graph.Edges) {
		auto CurrentChunk = *OtherChunkIter;
		if (!CurrentChunk->AddEdge(EdgeId)) {
			CurrentChunk = *(++OtherChunkIter);
			const bool Result = CurrentChunk->AddEdge(EdgeId);
			check(Result);
		}

		// use Insert if check below fails
		const auto LookupIdx = EdgesChunksLookup.Add(CurrentChunk);
		check(EdgeId.GetIndex() == LookupIdx);
	}

	// initialize filled chunks
	for (const auto GraphChunk : NewGraphChunks)
		GraphChunk->Initialize(GraphId);

	// since creating runtime mesh components (chunks) marks LODs and collisions implicitly,
	// we don't need to mark them ourselves again
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RemoveGraphChunks"), STAT_AGraphsRenderers_RemoveGraphChunks, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RemoveGraphChunks(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RemoveGraphChunks);
	const auto &Graph = ES::GetEntity<GraphEntity>(GraphId);

	// remove vertices chunks lookups
	for (const auto VertexId : Graph.Vertices) {
		check(VerticesChunksLookup.IsValidIndex(VertexId.GetIndex()));
		VerticesChunksLookup.RemoveAt(VertexId.GetIndex());
	}

	// remove edges chunks lookups
	for (const auto EdgeId : Graph.Edges) {
		check(EdgesChunksLookup.IsValidIndex(EdgeId.GetIndex()));
		EdgesChunksLookup.RemoveAt(EdgeId.GetIndex());
	}

	// remove graph chunks
	check(GraphsChunks.IsValidIndex(GraphId.GetIndex()));
	const auto &ChunksToRemove = GraphsChunks[GraphId.GetIndex()];
	for (const auto GraphChunk : ChunksToRemove) {
		const auto CheckNum = AllChunks.Remove(GraphChunk);
		check(CheckNum == 1);
		GraphChunk->Destroy();
	}
	GraphsChunks.RemoveAt(GraphId.GetIndex());

	// since removing runtime mesh components (chunks) clears LODs and collisions implicitly,
	// we don't need to clear them ourselves again
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RemoveVertexFromChunk"), STAT_AGraphsRenderers_RemoveVertexFromChunk, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RemoveVertexFromChunk(const EntityId VertexId) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RemoveVertexFromChunk);

	// remove from vertices lookup
	check(VerticesChunksLookup.IsValidIndex(VertexId.GetIndex()));
	const auto VertexChunk = VerticesChunksLookup[VertexId.GetIndex()];
	VerticesChunksLookup.RemoveAt(VertexId.GetIndex());

	// remove vertex id from chunk
	const auto Result = VertexChunk->RemoveVertex(VertexId);
	check(Result);

	const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);

	if (!VertexChunk->IsEmpty()) {
		VertexChunk->MarkAllVerticesDirty(true, true);
	}
	else {
		// chunk is empty -> remove it
		check(GraphsChunks.IsValidIndex(Vertex.GraphId.GetIndex()));
		auto CheckNum = GraphsChunks[Vertex.GraphId.GetIndex()].Remove(VertexChunk);
		check(CheckNum == 1);
		CheckNum = AllChunks.Remove(VertexChunk);
		check(CheckNum == 1);
		VertexChunk->Destroy();
		// since removing runtime mesh component (chunk) clears LODs and collisions implicitly,
		// we don't need to clear them ourselves again
	}

	// remove connected edges from chunks
	for (const auto EdgeId : Vertex.ConnectedEdges)
		RemoveEdgeFromChunk(EdgeId);
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RemoveEdgeFromChunk"), STAT_AGraphsRenderers_RemoveEdgeFromChunk, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RemoveEdgeFromChunk(const EntityId EdgeId) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RemoveEdgeFromChunk);

	// remove from lookup
	check(EdgesChunksLookup.IsValidIndex(EdgeId.GetIndex()));
	const auto EdgeChunk = EdgesChunksLookup[EdgeId.GetIndex()];
	EdgesChunksLookup.RemoveAt(EdgeId.GetIndex());

	// remove edge id from chunk
	const auto Result = EdgeChunk->RemoveEdge(EdgeId);
	check(Result);

	if (!EdgeChunk->IsEmpty()) {
		EdgeChunk->MarkAllEdgesDirty(true, true);
	}
	else {
		// chunk is empty -> remove it
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		check(GraphsChunks.IsValidIndex(Edge.GraphId.GetIndex()));
		auto CheckNum = GraphsChunks[Edge.GraphId.GetIndex()].Remove(EdgeChunk);
		check(CheckNum == 1);
		CheckNum = AllChunks.Remove(EdgeChunk);
		check(CheckNum == 1);
		EdgeChunk->Destroy();

		// since removing runtime mesh component (chunk) clears LODs and collisions implicitly,
		// we don't need to clear them ourselves again
	}
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::MarkGraphDirty"), STAT_AGraphsRenderers_MarkGraphDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::MarkGraphDirty(
	const EntityId GraphId,
	const bool MarkVertexMesh, const bool MarkVertexCollision,
	const bool MarkEdgeMesh, const bool MarkEdgeCollision
) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_MarkGraphDirty);
	check(GraphsChunks.IsValidIndex(GraphId.GetIndex()));

	const auto &ChunksToRemove = GraphsChunks[GraphId.GetIndex()];
	for (const auto GraphChunk : ChunksToRemove) {
		GraphChunk->MarkAllVerticesDirty(MarkVertexMesh, MarkVertexCollision);
		GraphChunk->MarkAllEdgesDirty(MarkEdgeMesh, MarkEdgeCollision);
	}
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::MarkVertexDirty"), STAT_AGraphsRenderers_MarkVertexDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::MarkVertexDirty(
	const EntityId VertexId,
	const bool MarkMesh, const bool MarkCollision,
	const bool MarkConnectedEdges
) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_MarkVertexDirty);

	check(VerticesChunksLookup.IsValidIndex(VertexId.GetIndex()));
	const auto VertexChunk = VerticesChunksLookup[VertexId.GetIndex()];
	VertexChunk->MarkAllVerticesDirty(MarkMesh, MarkCollision);

	if (MarkConnectedEdges) {
		const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
		for (const auto EdgeId : Vertex.ConnectedEdges)
			MarkEdgeDirty(EdgeId, MarkMesh, MarkCollision, false);
	}
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::MarkEdgeDirty"), STAT_AGraphsRenderers_MarkEdgeDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::MarkEdgeDirty(
	const EntityId EdgeId,
	const bool MarkMesh, const bool MarkCollision,
	const bool MarkConnectedVertices
) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_MarkEdgeDirty);
	if (!MarkConnectedVertices) {
		check(EdgesChunksLookup.IsValidIndex(EdgeId.GetIndex()));
		const auto EdgeChunk = EdgesChunksLookup[EdgeId.GetIndex()];
		EdgeChunk->MarkAllEdgesDirty(MarkMesh, MarkCollision);
	}
	else {
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		for (const auto VertexId : Edge.ConnectedVertices)
			MarkVertexDirty(VertexId, MarkMesh, MarkCollision, true);
	}
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RedrawGraphChunksIfDirty"), STAT_AGraphsRenderers_RedrawGraphChunksIfDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RedrawGraphChunksIfDirty(const EntityId GraphId) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RedrawGraphChunksIfDirty);
	check(GraphsChunks.IsValidIndex(GraphId.GetIndex()));

	const auto &GraphChunks = GraphsChunks[GraphId.GetIndex()];
	for (const auto GraphChunk : GraphChunks)
		GraphChunk->RedrawIfDirty();
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RedrawChunkByVertexIfDirty"), STAT_AGraphsRenderers_RedrawChunkByVertexIfDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RedrawChunkByVertexIfDirty(const EntityId VertexId, const bool RedrawConnectedEdges) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RedrawChunkByVertexIfDirty);

	check(VerticesChunksLookup.IsValidIndex(VertexId.GetIndex()));
	const auto VertexChunk = VerticesChunksLookup[VertexId.GetIndex()];
	VertexChunk->RedrawIfDirty();

	if (RedrawConnectedEdges) {
		const auto &Vertex = ES::GetEntity<VertexEntity>(VertexId);
		for (const auto EdgeId : Vertex.ConnectedEdges)
			RedrawChunkByEdgeIfDirty(EdgeId, false);
	}
}

DECLARE_CYCLE_STAT(TEXT("AGraphsRenderers::RedrawChunkByEdgeIfDirty"), STAT_AGraphsRenderers_RedrawChunkByEdgeIfDirty, GRAPHS_PERF_GRAPHS_RENDERERS);
void AGraphsRenderers::RedrawChunkByEdgeIfDirty(const EntityId EdgeId, const bool RedrawConnectedVertices) {
	SCOPE_CYCLE_COUNTER(STAT_AGraphsRenderers_RedrawChunkByEdgeIfDirty);
	if (!RedrawConnectedVertices) {
		check(EdgesChunksLookup.IsValidIndex(EdgeId.GetIndex()));
		const auto EdgeChunk = EdgesChunksLookup[EdgeId.GetIndex()];
		EdgeChunk->RedrawIfDirty();
	}
	else {
		const auto &Edge = ES::GetEntity<EdgeEntity>(EdgeId);
		for (const auto VertexId : Edge.ConnectedVertices)
			RedrawChunkByVertexIfDirty(VertexId, true);
	}
}
