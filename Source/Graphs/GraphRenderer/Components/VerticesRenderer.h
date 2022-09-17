#pragma once

#include "RuntimeMeshProvider.h"
#include "VerticesRenderer.generated.h"

struct VerticesRenderData {
	TArray<uint32_t> StorageIndices;
	TArray<FVector> Positions;
	TArray<FColor> Colors;
};

UCLASS()
class GRAPHS_API UVerticesRenderer final : public URuntimeMeshProvider {
	GENERATED_BODY()
public:
	UVerticesRenderer();
	virtual void Initialize() override;

	void SetRenderData(VerticesRenderData &&InRenderData, bool MarkLODs, bool MarkCollision);

	virtual FBoxSphereBounds GetBounds() override;
	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData &MeshData) override;

	virtual FRuntimeMeshCollisionSettings GetCollisionSettings() override;
	virtual bool HasCollisionMesh() override { return true; }
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) override;

	virtual bool IsThreadSafe() override { return true; }
private:
	static void GenerateEmptyCollision(FRuntimeMeshCollisionData &CollisionData);

	UPROPERTY()
	UMaterial *MeshMaterial;

	constexpr static uint32_t MeshQuality = 1;
	constexpr static float MeshScale = 50.0f;
	constexpr static uint32_t CollisionQuality = 0;

	mutable FCriticalSection PropertySyncRoot;
	VerticesRenderData RenderData;
};
