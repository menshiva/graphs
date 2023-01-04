#pragma once

#include "RendererBase.h"
#include "VerticesRenderer.generated.h"

UCLASS()
class GRAPHS_API UVerticesRenderer final : public URendererBase {
	GENERATED_BODY()
public:
	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData &MeshData) override;
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) override;
	constexpr static float MeshScale = 50.0f;
};
