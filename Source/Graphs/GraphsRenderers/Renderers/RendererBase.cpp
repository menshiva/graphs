#include "RendererBase.h"
#include "VerticesRenderer.h"

URendererBase::URendererBase() {
	const static ConstructorHelpers::FObjectFinder<UMaterial> GraphMaterialAsset(
		TEXT("/Game/Graphs/Materials/GraphMaterial")
	);
	MeshMaterial = GraphMaterialAsset.Object;
}

void URendererBase::SetInitRenderData(RenderData &&InRenderData) {
	Data = MoveTemp(InRenderData);
	check(InRenderData.StorageIds.Num() == 0 && InRenderData.Positions.Num() == 0 && InRenderData.Colors.Num() == 0);
}

void URendererBase::Initialize() {
	Super::Initialize();
	SetupMaterialSlot(0, "Mesh Material", MeshMaterial);

	// TODO: more LODs?
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	ConfigureLODs({LODProperties});

	FRuntimeMeshSectionProperties Properties;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Average;
	Properties.bUseHighPrecisionTangents = false;
	Properties.bUseHighPrecisionTexCoords = false;
	Properties.bWants32BitIndices = true;
	Properties.bCastsShadow = false;
	Properties.bForceOpaque = true;
	CreateSection(0, 0, Properties);
}

FBoxSphereBounds URendererBase::GetBounds() {
	FBox GraphsBounds(ForceInitToZero);
	for (const auto &VertexPos : Data.Positions)
		GraphsBounds += VertexPos;
	return GraphsBounds.ExpandBy(UVerticesRenderer::MeshScale);
}

void URendererBase::SetRenderData(RenderData &&InRenderData, const bool MarkLODs, const bool MarkCollision) {
	check(MarkLODs || MarkCollision);
	{
		FScopeLock Lock(&DataSyncRoot);
		Data = MoveTemp(InRenderData);
		check(InRenderData.StorageIds.Num() == 0 && InRenderData.Positions.Num() == 0 && InRenderData.Colors.Num() == 0);
	}
	if (MarkLODs)
		MarkSectionDirty(0, 0);
	if (MarkCollision)
		MarkCollisionDirty();
}

FRuntimeMeshCollisionSettings URendererBase::GetCollisionSettings() {
	FRuntimeMeshCollisionSettings CollisionSettings;
	CollisionSettings.CookingMode = ERuntimeMeshCollisionCookingMode::CookingPerformance;
	CollisionSettings.bUseComplexAsSimple = true;
	CollisionSettings.bUseAsyncCooking = true;
	return CollisionSettings;
}

void URendererBase::GenerateEmptyCollision(FRuntimeMeshCollisionData &CollisionData) {
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	CollisionData.Vertices.Reserve(3);
	CollisionData.Vertices.Add(FVector(100000.000f));
	CollisionData.Vertices.Add(FVector(100000.001f));
	CollisionData.Vertices.Add(FVector(100000.002f));
	CollisionData.Triangles.Add(0, 1, 2);
}
