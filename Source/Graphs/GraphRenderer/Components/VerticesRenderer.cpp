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
		if (Data.Positions.Num() == 0)
			return false;
		TmpData = Data;
	}

	check(TmpData.StorageIndices.Num() == TmpData.Positions.Num());
	check(TmpData.Positions.Num() == TmpData.Colors.Num());
	check(MeshData.Positions.Num() == 0);
	check(MeshData.Triangles.Num() == 0);

	constexpr static auto IcosahedronScaled = VertexMeshFactory::GenerateScaled<MeshQuality, MeshScale>();
	static_assert(IcosahedronScaled.Vertices[0].X == -0.525731087f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Y == 0.850650787f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Z == 0.0f * MeshScale);

	const size_t VerticesNum = IcosahedronScaled.Vertices.size() * TmpData.Positions.Num();
	const size_t IndicesNum = IcosahedronScaled.Indices.size() * TmpData.Positions.Num();

	TArray<FVector> TmpVertices;
	TmpVertices.SetNumUninitialized(VerticesNum);
	TArray<FColor> TmpColors;
	TmpColors.SetNumUninitialized(VerticesNum);
	TArray<int32> TmpIndices;
	TmpIndices.SetNumUninitialized(IndicesNum);

	const auto IcosahedronFVecVertices = reinterpret_cast<const FVector*>(IcosahedronScaled.Vertices.data());

	for (size_t RdataI = 0; RdataI < TmpData.StorageIndices.Num(); ++RdataI) {
		const auto &VertexPos = TmpData.Positions[RdataI];
		const auto &VertexColor = TmpData.Colors[RdataI];

		const size_t PrevVerticesNum = RdataI * IcosahedronScaled.Vertices.size();
		const size_t CurrVerticesNum = (RdataI + 1) * IcosahedronScaled.Vertices.size();
		for (size_t i = PrevVerticesNum; i < CurrVerticesNum; ++i) {
			TmpVertices[i] = IcosahedronFVecVertices[i - PrevVerticesNum] + VertexPos;
			TmpColors[i] = VertexColor;
		}

		const size_t PrevIndicesNum = RdataI * IcosahedronScaled.Indices.size();
		const size_t CurrIndicesNum = (RdataI + 1) * IcosahedronScaled.Indices.size();
		for (size_t i = PrevIndicesNum; i < CurrIndicesNum; ++i)
			TmpIndices[i] = IcosahedronScaled.Indices[i - PrevIndicesNum] + PrevVerticesNum;
	}

	MeshData.Positions.Append(TmpVertices);
	MeshData.Tangents.SetNum(VerticesNum);
	MeshData.TexCoords.SetNum(VerticesNum);
	MeshData.Colors.Append(TmpColors);
	MeshData.Triangles.Append(TmpIndices);

	return true;
}

bool UVerticesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetCollisionMesh);

	check(Data.StorageIndices.Num() == Data.Positions.Num());
	check(Data.Positions.Num() == Data.Colors.Num());
	check(CollisionData.Vertices.Num() == 0);
	check(CollisionData.Triangles.Num() == 0);
	check(CollisionData.CollisionSources.Num() == 0);

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	constexpr static auto IcosahedronScaled = VertexMeshFactory::GenerateScaled<CollisionQuality, MeshScale>();
	static_assert(IcosahedronScaled.Vertices[0].X == -0.525731087f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Y == 0.850650787f * MeshScale);
	static_assert(IcosahedronScaled.Vertices[0].Z == 0.0f * MeshScale);

	const size_t VerticesNum = IcosahedronScaled.Vertices.size() * Data.Positions.Num();
	const size_t TrianglesNum = IcosahedronScaled.Indices.size() / 3 * Data.Positions.Num();
	const size_t SourcesNum = Data.Positions.Num();

	CollisionData.Vertices.Reserve(VerticesNum);
	CollisionData.Triangles.Reserve(TrianglesNum);
	CollisionData.CollisionSources.Reserve(SourcesNum);

	const auto IcosahedronFVecVertices = reinterpret_cast<const FVector*>(IcosahedronScaled.Vertices.data());

	for (size_t RdataI = 0; RdataI < Data.StorageIndices.Num(); ++RdataI) {
		const auto VertexIdx = Data.StorageIndices[RdataI];
		const auto &VertexPos = Data.Positions[RdataI];

		const auto PrevVerticesNum = CollisionData.Vertices.Num();
		for (size_t i = 0; i < IcosahedronScaled.Vertices.size(); ++i)
			CollisionData.Vertices.Add(VertexPos + IcosahedronFVecVertices[i]);

		const int32_t TrianglesStart = CollisionData.Triangles.Num();
		check(IcosahedronScaled.Indices.size() % 3 == 0);
		for (size_t i = 0; i < IcosahedronScaled.Indices.size(); i += 3) {
			CollisionData.Triangles.Add(
				IcosahedronScaled.Indices[i + 0] + PrevVerticesNum,
				IcosahedronScaled.Indices[i + 1] + PrevVerticesNum,
				IcosahedronScaled.Indices[i + 2] + PrevVerticesNum
			);
		}
		const int32_t TrianglesEnd = CollisionData.Triangles.Num() - 1;

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
