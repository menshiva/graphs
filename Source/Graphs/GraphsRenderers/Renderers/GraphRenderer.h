#pragma once

#include "RuntimeMeshComponent.h"
#include "Graphs/EntityStorage/EntityStorage.h"
#include "EdgesRenderer.h"
#include "VerticesRenderer.h"
#include <bitset>
#include "GraphRenderer.generated.h"

struct DirtyComponentInfo {
	EntitySignature ComponentType;
	bool MarkMesh;
	bool MarkCollision;
};

UCLASS()
class GRAPHS_API AGraphRenderer final : public AActor {
	GENERATED_BODY()
public:
	explicit AGraphRenderer(const FObjectInitializer &ObjectInitializer);
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	FORCEINLINE EntityId GetGraphId() const { return GraphId; }

	void MarkDirty(DirtyComponentInfo &&Info);
private:
	void Initialize(EntityId InGraphId);
	void RedrawIfDirty();

	EntityId GetEntityIdFromHitResult(const FHitResult &HitResult) const;

	RenderData GenerateVerticesRenderData(const GraphEntity &Graph) const;
	RenderData GenerateEdgesRenderData(const GraphEntity &Graph) const;

	EntityId GraphId = EntityId::NONE();

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

	friend class AGraphsRenderers;
};
