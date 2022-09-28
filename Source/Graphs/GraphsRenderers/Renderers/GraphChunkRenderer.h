#pragma once

#include "RuntimeMeshComponent.h"
#include "Graphs/EntityStorage/EntityStorage.h"
#include "EdgesRenderer.h"
#include "VerticesRenderer.h"
#include <bitset>
#include "GraphChunkRenderer.generated.h"

UCLASS()
class GRAPHS_API AGraphChunkRenderer final : public AActor {
	GENERATED_BODY()
public:
	explicit AGraphChunkRenderer(const FObjectInitializer &ObjectInitializer);
	void Initialize(EntityId InGraphId);

	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;
	FORCEINLINE EntityId GetGraphId() const { return GraphId; }

	FORCEINLINE bool IsEmpty() const { return VerticesToDraw.Num() == 0 && EdgesToDraw.Num() == 0; }

	bool AddVertex(EntityId VertexId);
	bool AddEdge(EntityId EdgeId);

	void MarkAllVerticesDirty(bool MarkMesh, bool MarkCollision);
	void MarkAllEdgesDirty(bool MarkMesh, bool MarkCollision);

	bool RemoveVertex(EntityId VertexId);
	bool RemoveEdge(EntityId EdgeId);

	void RedrawIfDirty();

	// number of entities rendered by 1 graph chunk renderer
	constexpr static size_t ChunkSize = 256;
private:
	RenderData GenerateVerticesRenderData(const GraphEntity &Graph) const;
	RenderData GenerateEdgesRenderData(const GraphEntity &Graph) const;

	EntityId GraphId = EntityId::NONE();
	TSet<EntityId, DefaultKeyFuncs<EntityId>, TFixedSetAllocator<ChunkSize>> VerticesToDraw;
	TSet<EntityId, DefaultKeyFuncs<EntityId>, TFixedSetAllocator<ChunkSize>> EdgesToDraw;

	UPROPERTY()
	URuntimeMeshComponent *VerticesRuntimeMeshComponent;

	UPROPERTY()
	UVerticesRenderer *VerticesProvider;

	UPROPERTY()
	URuntimeMeshComponent *EdgesRuntimeMeshComponent;

	UPROPERTY()
	UEdgesRenderer *EdgesProvider;

	std::bitset<GRAPH> MeshDirty;
	std::bitset<GRAPH> CollisionDirty;
};
