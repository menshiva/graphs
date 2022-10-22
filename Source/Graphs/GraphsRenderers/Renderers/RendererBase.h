#pragma once

#include "RuntimeMeshProvider.h"
#include "RendererBase.generated.h"

struct RenderData {
	TArray<uint32_t> StorageIds;
	TArray<FVector> Positions;
	TArray<FColor> Colors;
};

UCLASS(Abstract)
class GRAPHS_API URendererBase : public URuntimeMeshProvider {
	GENERATED_BODY()
public:
	virtual void Initialize() override;
	virtual FBoxSphereBounds GetBounds() override;

	FORCEINLINE void SetInitRenderData(RenderData &&InRenderData) { Data = MoveTemp(InRenderData); }
	void SetRenderData(RenderData &&InRenderData, bool MarkLODs, bool MarkCollision);

	virtual FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	virtual bool HasCollisionMesh() override { return true; }

	virtual bool IsThreadSafe() override { return true; }
protected:
	static void GenerateEmptyCollision(FRuntimeMeshCollisionData &CollisionData);

	mutable FCriticalSection DataSyncRoot;
	RenderData Data;
};
