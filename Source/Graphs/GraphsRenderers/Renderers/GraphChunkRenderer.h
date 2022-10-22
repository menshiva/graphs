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
	FORCEINLINE bool IsEmpty() const { return VerticesToDraw.Num() == 0 && EdgesToDraw.Num() == 0; }

	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;

	bool AddVertex(EntityId VertexId);
	bool AddEdge(EntityId EdgeId);

	void MarkAllVerticesDirty(bool MarkMesh, bool MarkCollision);
	void MarkAllEdgesDirty(bool MarkMesh, bool MarkCollision);

	FORCEINLINE bool RemoveVertex(const EntityId VertexId) { return VerticesToDraw.Remove(VertexId) == 1; }
	FORCEINLINE bool RemoveEdge(const EntityId EdgeId) { return EdgesToDraw.Remove(EdgeId) == 1; }

	void RedrawIfDirty();

	// number of entities rendered by 1 graph chunk renderer
	constexpr static size_t ChunkSize = 128;
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
