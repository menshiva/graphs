#pragma once

#include "RendererBase.h"
#include "Graphs/Utils/VertexMeshFactory.h"
#include "VerticesRenderer.generated.h"

UCLASS()
class GRAPHS_API UVerticesRenderer final : public URendererBase {
	GENERATED_BODY()
public:
	virtual void Initialize() override;

	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData &MeshData) override;
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) override;

	constexpr static uint32_t MeshQuality = 1;
	constexpr static float MeshScale = 50.0f;

	constexpr static uint32_t CollisionQuality = 0;
private:
	constexpr static auto IcosahedronMesh = VertexMeshFactory::GenerateScaled<MeshQuality, MeshScale>();
	constexpr static auto IcosahedronCollision = VertexMeshFactory::GenerateScaled<CollisionQuality, MeshScale>();
};
