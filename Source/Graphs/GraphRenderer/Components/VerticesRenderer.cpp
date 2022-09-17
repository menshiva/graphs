#include "VerticesRenderer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MeshFactory.h"

UVerticesRenderer::UVerticesRenderer() {
	const static ConstructorHelpers::FObjectFinder<UMaterial> GraphMaterialAsset(
		TEXT("/Game/Graphs/Materials/GraphMaterial")
	);
	MeshMaterial = GraphMaterialAsset.Object;
}

void UVerticesRenderer::Initialize() {
	Super::Initialize();
	SetupMaterialSlot(0, "Vertex Mesh Material", MeshMaterial);

	// TODO: more LODs?
	FRuntimeMeshLODProperties LODProperties;
	LODProperties.ScreenSize = 0.0f;
	ConfigureLODs({LODProperties});

	FRuntimeMeshSectionProperties Properties;
	Properties.bIsVisible = true;
	Properties.MaterialSlot = 0;
	Properties.UpdateFrequency = ERuntimeMeshUpdateFrequency::Average;
	Properties.bUseHighPrecisionTangents = false; // we don't use it
	Properties.bUseHighPrecisionTexCoords = false; // we don't use it
	// since 1 vertex mesh can be consisted of 642 vertices, we will probably overflow 2^16
	Properties.bWants32BitIndices = true;
	Properties.bCastsShadow = false;
	Properties.bForceOpaque = true;
	CreateSection(0, 0, Properties);
}

FBoxSphereBounds UVerticesRenderer::GetBounds() {
	FBoxSphereBounds GraphsBounds(ForceInitToZero);
	for (const auto &VertexPos : RenderData.Positions)
		GraphsBounds = GraphsBounds + FBoxSphereBounds(FSphere(VertexPos, MeshScale));
	return GraphsBounds;
}

void UVerticesRenderer::SetRenderData(VerticesRenderData &&InRenderData, const bool MarkLODs, const bool MarkCollision) {
	check(MarkLODs || MarkCollision);
	FScopeLock Lock(&PropertySyncRoot);
	RenderData = MoveTemp(InRenderData);
	if (MarkLODs)
		MarkAllLODsDirty();
	if (MarkCollision)
		MarkCollisionDirty();
}

bool UVerticesRenderer::GetSectionMeshForLOD(
	const int32 LODIndex,
	const int32 SectionId,
	FRuntimeMeshRenderableMeshData &MeshData
) {
	check(LODIndex == 0 && SectionId == 0);
	UKismetSystemLibrary::PrintString(GetWorld(), "UVertexProvider::GetSectionMeshForLOD", true, true, FLinearColor::Red);

	VerticesRenderData TmpRenderData;
	{
		FScopeLock Lock(&PropertySyncRoot);
		TmpRenderData = RenderData;
	}

	if (TmpRenderData.Positions.Num() == 0)
		return false;
	check(TmpRenderData.Positions.Num() == TmpRenderData.Colors.Num());

	constexpr static auto Icosahedron = VertexMeshFactory::GenerateUnit<MeshQuality>();
	static_assert(Icosahedron.Vertices[0].X == -0.525731087f);
	static_assert(Icosahedron.Vertices[0].Y == 0.850650787f);
	static_assert(Icosahedron.Vertices[0].Z == 0.0f);

	const size_t VerticesNum = MeshData.Positions.Num()
		+ VertexMeshFactory::GetVerticesNum(MeshQuality) * TmpRenderData.Positions.Num();
	const size_t IndicesNum = MeshData.Triangles.Num()
		+ VertexMeshFactory::GetIndicesNum(MeshQuality) * TmpRenderData.Positions.Num();

	MeshData.ReserveVertices(VerticesNum);
	MeshData.Triangles.Reserve(IndicesNum);

	for (size_t RdataI = 0; RdataI < TmpRenderData.Positions.Num(); ++RdataI) {
		const auto &VertexPos = TmpRenderData.Positions[RdataI];
		const auto &VertexColor = TmpRenderData.Colors[RdataI];

		const auto VerticesOffset = MeshData.Positions.Num();
		for (size_t i = 0; i < Icosahedron.Vertices.size(); ++i) {
			const auto &ConvertedUnitVec = *reinterpret_cast<const FVector*>(&Icosahedron.Vertices[i]);
			MeshData.Positions.Add(VertexPos + ConvertedUnitVec * MeshScale);
			MeshData.Tangents.Add(FVector::UpVector, FVector::RightVector);
			MeshData.TexCoords.Add(FVector2D::UnitVector);
			MeshData.Colors.Add(VertexColor);
		}

		check(Icosahedron.Indices.size() % 3 == 0);
		for (const auto Idx : Icosahedron.Indices)
			MeshData.Triangles.Add(Idx + VerticesOffset);
	}

	check(MeshData.Positions.Num() == VerticesNum);
	check(MeshData.Tangents.Num() == VerticesNum);
	check(MeshData.TexCoords.Num() == VerticesNum);
	check(MeshData.Colors.Num() == VerticesNum);
	check(MeshData.Triangles.Num() == IndicesNum);

	return true;
}

FRuntimeMeshCollisionSettings UVerticesRenderer::GetCollisionSettings() {
	FRuntimeMeshCollisionSettings CollisionSettings;
	CollisionSettings.CookingMode = ERuntimeMeshCollisionCookingMode::CookingPerformance;
	CollisionSettings.bUseComplexAsSimple = true;
	CollisionSettings.bUseAsyncCooking = true;
	return CollisionSettings;
}

bool UVerticesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	check(RenderData.Positions.Num() == RenderData.Colors.Num());
	UKismetSystemLibrary::PrintString(GetWorld(), "UVertexProvider::GetCollisionMesh", true, true, FLinearColor::Red);

	if (RenderData.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	constexpr static auto Icosahedron = VertexMeshFactory::GenerateUnit<CollisionQuality>();
	static_assert(Icosahedron.Vertices[0].X == -0.525731087f);
	static_assert(Icosahedron.Vertices[0].Y == 0.850650787f);
	static_assert(Icosahedron.Vertices[0].Z == 0.0f);

	const size_t VerticesNum = CollisionData.Vertices.Num()
		+ VertexMeshFactory::GetVerticesNum(CollisionQuality) * RenderData.Positions.Num();
	const size_t TrianglesNum = CollisionData.Triangles.Num()
		+ VertexMeshFactory::GetIndicesNum(CollisionQuality) / 3 * RenderData.Positions.Num();
	const size_t SourcesNum = CollisionData.CollisionSources.Num() + RenderData.Positions.Num();

	CollisionData.Vertices.Reserve(VerticesNum);
	CollisionData.Triangles.Reserve(TrianglesNum);
	CollisionData.CollisionSources.Reserve(SourcesNum);

	for (size_t RdataI = 0; RdataI < RenderData.Positions.Num(); ++RdataI) {
		const auto VertexIdx = RenderData.StorageIndices[RdataI];
		const auto &VertexPos = RenderData.Positions[RdataI];

		const auto VerticesOffset = CollisionData.Vertices.Num();
		for (size_t i = 0; i < Icosahedron.Vertices.size(); ++i) {
			const auto &ConvertedUnitVec = *reinterpret_cast<const FVector*>(&Icosahedron.Vertices[i]);
			CollisionData.Vertices.Add(VertexPos + ConvertedUnitVec * MeshScale);
		}

		const int32_t TrianglesStart = CollisionData.Triangles.Num();
		check(Icosahedron.Indices.size() % 3 == 0);
		for (size_t i = 0; i < Icosahedron.Indices.size(); i += 3) {
			CollisionData.Triangles.Add(
				Icosahedron.Indices[i + 0] + VerticesOffset,
				Icosahedron.Indices[i + 1] + VerticesOffset,
				Icosahedron.Indices[i + 2] + VerticesOffset
			);
		}
		const int32_t TrianglesEnd = CollisionData.Triangles.Num();

		CollisionData.CollisionSources.Emplace(
			TrianglesStart, TrianglesEnd,
			this, VertexIdx,
			ERuntimeMeshCollisionFaceSourceType::Collision
		);
	}

	check(CollisionData.Vertices.Num() == VerticesNum);
	check(CollisionData.Triangles.Num() == TrianglesNum);
	check(CollisionData.CollisionSources.Num() == SourcesNum);

	return true;
}

void UVerticesRenderer::GenerateEmptyCollision(FRuntimeMeshCollisionData &CollisionData) {
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	CollisionData.Vertices.Reserve(3);
	CollisionData.Vertices.Add(FVector(100000.000f));
	CollisionData.Vertices.Add(FVector(100000.001f));
	CollisionData.Vertices.Add(FVector(100000.002f));
	CollisionData.Triangles.Add(0, 1, 2);
}
