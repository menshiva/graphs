#include "VerticesRenderer.h"
#include "Graphs/Utils/Utils.h"
#include "Graphs/Utils/VertexMeshFactory.h"

DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetSectionMeshForLOD"), STAT_UVerticesRenderer_GetSectionMeshForLOD, GRAPHS_PERF_VERTICES_RENDERER);
DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetCollisionMesh"), STAT_UVerticesRenderer_GetCollisionMesh, GRAPHS_PERF_VERTICES_RENDERER);

UVerticesRenderer::UVerticesRenderer() : URendererBase() {}

bool UVerticesRenderer::GetSectionMeshForLOD(
	const int32 LODIndex,
	const int32 SectionId,
	FRuntimeMeshRenderableMeshData &MeshData
) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetSectionMeshForLOD);
	check(LODIndex == 0 && SectionId == 0);

	RenderData TmpData;
	{
		FScopeLock Lock(&DataSyncRoot);
		TmpData.Positions = Data.Positions;
		TmpData.Colors = Data.Colors;
	}

	if (TmpData.Positions.Num() == 0)
		return false;

	check(TmpData.Positions.Num() == TmpData.Colors.Num());
	check(MeshData.Positions.Num() == 0);
	check(MeshData.Triangles.Num() == 0);

	constexpr static auto IcosahedronScaled = VertexMeshFactory::GenerateScaled<MeshQuality, MeshScale>();
	const static auto IcosahedronFVecVertices = reinterpret_cast<const FVector*>(IcosahedronScaled.Vertices.data());
	static_assert(IcosahedronScaled.Vertices[0].X == -0.525731087f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Y == 0.850650787f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Z == 0.0f * MeshScale);

	const size_t VerticesNum = IcosahedronScaled.Vertices.size() * TmpData.Positions.Num();
	const size_t IndicesNum = IcosahedronScaled.Indices.size() * TmpData.Positions.Num();

	MeshData.Positions.SetNum(VerticesNum);
	MeshData.Tangents.SetNum(VerticesNum);
	MeshData.TexCoords.SetNum(VerticesNum);
	MeshData.Colors.SetNum(VerticesNum);
	MeshData.Triangles.SetNum(IndicesNum);

	for (size_t RdataI = 0; RdataI < TmpData.Positions.Num(); ++RdataI) {
		const auto &VertexPos = TmpData.Positions[RdataI];
		const auto &VertexColor = TmpData.Colors[RdataI];

		const size_t PrevVerticesNum = RdataI * IcosahedronScaled.Vertices.size();
		const size_t CurrVerticesNum = PrevVerticesNum + IcosahedronScaled.Vertices.size();
		for (size_t i = PrevVerticesNum; i < CurrVerticesNum; ++i)
			MeshData.Positions.SetPosition(i, IcosahedronFVecVertices[i - PrevVerticesNum] + VertexPos);
		for (size_t i = PrevVerticesNum; i < CurrVerticesNum; ++i)
			MeshData.Colors.SetColor(i, VertexColor);

		const size_t PrevIndicesNum = RdataI * IcosahedronScaled.Indices.size();
		const size_t CurrIndicesNum = PrevIndicesNum + IcosahedronScaled.Indices.size();
		for (size_t i = PrevIndicesNum; i < CurrIndicesNum; ++i)
			MeshData.Triangles.SetVertexIndex(i, IcosahedronScaled.Indices[i - PrevIndicesNum] + PrevVerticesNum);
	}

	return true;
}

bool UVerticesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetCollisionMesh);

	check(Data.StorageIds.Num() == Data.Positions.Num());
	check(Data.Positions.Num() == Data.StorageIds.Num());
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	check(CollisionData.CollisionSources.Num() == 0);

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	constexpr static auto IcosahedronScaled = VertexMeshFactory::GenerateScaled<CollisionQuality, MeshScale>();
	const static auto IcosahedronFVecVertices = reinterpret_cast<const FVector*>(IcosahedronScaled.Vertices.data());
	static_assert(IcosahedronScaled.Vertices[0].X == -0.525731087f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Y == 0.850650787f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Z == 0.0f * MeshScale);

	const size_t VerticesNum = IcosahedronScaled.Vertices.size() * Data.Positions.Num();
	const size_t TrianglesNum = IcosahedronScaled.Indices.size() / 3 * Data.Positions.Num();
	const size_t SourcesNum = Data.Positions.Num();

	CollisionData.Vertices.SetNum(VerticesNum);
	CollisionData.Triangles.SetNum(TrianglesNum);
	CollisionData.CollisionSources.SetNumUninitialized(SourcesNum);

	for (size_t RdataI = 0; RdataI < Data.Positions.Num(); ++RdataI) {
		const auto VertexId = Data.StorageIds[RdataI];
		const auto &VertexPos = Data.Positions[RdataI];

		const size_t PrevVerticesNum = RdataI * IcosahedronScaled.Vertices.size();
		const size_t CurrVerticesNum = PrevVerticesNum + IcosahedronScaled.Vertices.size();
		for (size_t i = PrevVerticesNum; i < CurrVerticesNum; ++i)
			CollisionData.Vertices.SetPosition(i, IcosahedronFVecVertices[i - PrevVerticesNum] + VertexPos);

		check(IcosahedronScaled.Indices.size() % 3 == 0);
		const size_t PrevTrianglesNum = RdataI * IcosahedronScaled.Indices.size() / 3;
		const size_t CurrTrianglesNum = PrevTrianglesNum + IcosahedronScaled.Indices.size() / 3;
		for (size_t i = PrevTrianglesNum; i < CurrTrianglesNum; ++i) {
			const size_t IcosahedronIndexIdx = (i - PrevTrianglesNum) * 3;
			CollisionData.Triangles.SetTriangleIndices(
				i,
				IcosahedronScaled.Indices[IcosahedronIndexIdx + 0] + PrevVerticesNum,
				IcosahedronScaled.Indices[IcosahedronIndexIdx + 1] + PrevVerticesNum,
				IcosahedronScaled.Indices[IcosahedronIndexIdx + 2] + PrevVerticesNum
			);
		}

		CollisionData.CollisionSources[RdataI] = FRuntimeMeshCollisionSourceSectionInfo(
			PrevTrianglesNum, CurrTrianglesNum - 1,
			this, EntityId::Hash(VertexId),
			ERuntimeMeshCollisionFaceSourceType::Collision
		);
	}

	return true;
}
