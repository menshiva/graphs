#pragma once

#include "RendererBase.h"
#include "EdgesRenderer.generated.h"

UCLASS()
class GRAPHS_API UEdgesRenderer final : public URendererBase {
	GENERATED_BODY()
public:
	UEdgesRenderer();

	virtual void Initialize() override;

	virtual bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRuntimeMeshRenderableMeshData &MeshData) override;
	virtual bool GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) override;

	constexpr static int32 MeshQuality = 4;
	constexpr static float MeshScale = 10.0f;

	constexpr static int32 CollisionQuality = 3;
private:
	UPROPERTY()
	UMaterialInstance *MeshMaterial;
};
