#pragma once

#include "Renderers/GraphChunkRenderer.h"
#include "GraphsRenderers.generated.h"

UCLASS()
class GRAPHS_API AGraphsRenderers final : public AActor {
	GENERATED_BODY()
public:
	AGraphsRenderers();
	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;

	void ConstructGraphChunks(EntityId GraphId);
	// TODO: void AddVertexToChunk(EntityId VertexId);
	// TODO: void AddEdgeToChunk(EntityId EdgeId);

	void RemoveGraphChunks(EntityId GraphId);
	void RemoveVertexFromChunk(EntityId VertexId);
	void RemoveEdgeFromChunk(EntityId EdgeId);

	void MarkGraphDirty(
		EntityId GraphId,
		bool MarkVertexMesh, bool MarkVertexCollision,
		bool MarkEdgeMesh, bool MarkEdgeCollision
	);
	void MarkVertexDirty(EntityId VertexId, bool MarkMesh, bool MarkCollision, bool MarkConnectedEdges);
	void MarkEdgeDirty(EntityId EdgeId, bool MarkMesh, bool MarkCollision, bool MarkConnectedVertices);

	void RedrawGraphChunksIfDirty(EntityId GraphId);
	void RedrawChunkByVertexIfDirty(EntityId VertexId, bool RedrawConnectedEdges);
	void RedrawChunkByEdgeIfDirty(EntityId EdgeId, bool RedrawConnectedVertices);
private:
	UPROPERTY()
	TSet<AGraphChunkRenderer*> AllChunks;

	TSparseArray<TSet<AGraphChunkRenderer*>> GraphsChunks;
	TSparseArray<AGraphChunkRenderer*> VerticesChunksLookup;
	TSparseArray<AGraphChunkRenderer*> EdgesChunksLookup;
};
