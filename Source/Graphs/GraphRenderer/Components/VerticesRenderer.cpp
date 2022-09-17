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

	constexpr static auto Icosahedron = VertexMeshFactory::GenerateUnit<MeshQuality>();
	static_assert(Icosahedron.Vertices[0].X == -0.525731087f);
	static_assert(Icosahedron.Vertices[0].Y == 0.850650787f);
	static_assert(Icosahedron.Vertices[0].Z == 0.0f);

	const size_t VerticesNum = MeshData.Positions.Num() + Icosahedron.Vertices.size() * TmpData.Positions.Num();
	const size_t IndicesNum = MeshData.Triangles.Num() + Icosahedron.Indices.size() * TmpData.Positions.Num();

	MeshData.ReserveVertices(VerticesNum);
	MeshData.Triangles.Reserve(IndicesNum);

	for (size_t RdataI = 0; RdataI < TmpData.Positions.Num(); ++RdataI) {
		const auto &VertexPos = TmpData.Positions[RdataI];
		const auto &VertexColor = TmpData.Colors[RdataI];

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

bool UVerticesRenderer::GetCollisionMesh(FRuntimeMeshCollisionData &CollisionData) {
	SCOPE_CYCLE_COUNTER(STAT_UVerticesRenderer_GetCollisionMesh);

	check(Data.StorageIndices.Num() == Data.Positions.Num());
	check(Data.Positions.Num() == Data.Colors.Num());

	if (Data.Positions.Num() == 0) {
		// hides collision if no data was provided
		GenerateEmptyCollision(CollisionData);
		return true;
	}

	constexpr static auto Icosahedron = VertexMeshFactory::GenerateUnit<CollisionQuality>();
	static_assert(Icosahedron.Vertices[0].X == -0.525731087f);
	static_assert(Icosahedron.Vertices[0].Y == 0.850650787f);
	static_assert(Icosahedron.Vertices[0].Z == 0.0f);

	const size_t VerticesNum = CollisionData.Vertices.Num() + Icosahedron.Vertices.size() * Data.Positions.Num();
	const size_t TrianglesNum = CollisionData.Triangles.Num() + Icosahedron.Indices.size() / 3 * Data.Positions.Num();
	const size_t SourcesNum = CollisionData.CollisionSources.Num() + Data.Positions.Num();

	CollisionData.Vertices.Reserve(VerticesNum);
	CollisionData.Triangles.Reserve(TrianglesNum);
	CollisionData.CollisionSources.Reserve(SourcesNum);

	for (size_t RdataI = 0; RdataI < Data.Positions.Num(); ++RdataI) {
		const auto VertexIdx = Data.StorageIndices[RdataI];
		const auto &VertexPos = Data.Positions[RdataI];

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
