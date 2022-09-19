#pragma once

#include "../EntityStorage/EntityId.h"
#include "RuntimeMeshProvider.h"
#include "RendererBase.generated.h"

struct RenderData {
	TArray<EntityId> StorageIds;
	TArray<FVector> Positions;
	TArray<FColor> Colors;
};

UCLASS()
class GRAPHS_API URendererBase : public URuntimeMeshProvider {
	GENERATED_BODY()
public:
	URendererBase();
	virtual void Initialize() override;

	virtual FBoxSphereBounds GetBounds() override;
	void SetRenderData(RenderData &&InRenderData, bool MarkLODs, bool MarkCollision);

	virtual FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	virtual bool HasCollisionMesh() override { return true; }

	virtual bool IsThreadSafe() override { return true; }
private:
	UPROPERTY()
	UMaterial *MeshMaterial;
protected:
	static void GenerateEmptyCollision(FRuntimeMeshCollisionData &CollisionData);

	mutable FCriticalSection DataSyncRoot;
	RenderData Data;
};
