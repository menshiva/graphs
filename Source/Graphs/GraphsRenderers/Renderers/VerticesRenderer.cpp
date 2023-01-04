#include "VerticesRenderer.h"
#include "Graphs/Utils/Utils.h"

DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetSectionMeshForLOD"), STAT_UVerticesRenderer_GetSectionMeshForLOD, GRAPHS_PERF_VERTICES_RENDERER);
DECLARE_CYCLE_STAT(TEXT("UVerticesRenderer::GetCollisionMesh"), STAT_UVerticesRenderer_GetCollisionMesh, GRAPHS_PERF_VERTICES_RENDERER);

constexpr static uint32_t IcosahedronFirstVertNum = 42;
constexpr static uint32_t IcosahedronFirstIndNum = 240;

constexpr static uint32_t IcosahedronZeroVertNum = 12;
constexpr static uint32_t IcosahedronZeroIndNum = 80;

/**
 * Hard-coded icosahedron of 1 order scaled by MeshScale.
 * Used for vertex collision.
 * 
 * @link https://observablehq.com/@mourner/fast-icosphere-mesh
 */
static std::pair<
	TArray<FVector, TFixedAllocator<IcosahedronFirstVertNum>>,
	TArray<int32, TFixedAllocator<IcosahedronFirstIndNum>>
> IcosahedronMesh = {
	{
		FVector(-0.525731, 0.850651, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(0.525731, 0.850651, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(-0.525731, -0.850651, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(0.525731, -0.850651, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(0.000000, -0.525731, 0.850651) * UVerticesRenderer::MeshScale,
		FVector(0.000000, 0.525731, 0.850651) * UVerticesRenderer::MeshScale,
		FVector(0.000000, -0.525731, -0.850651) * UVerticesRenderer::MeshScale,
		FVector(0.000000, 0.525731, -0.850651) * UVerticesRenderer::MeshScale,
		FVector(0.850651, 0.000000, -0.525731) * UVerticesRenderer::MeshScale,
		FVector(0.850651, 0.000000, 0.525731) * UVerticesRenderer::MeshScale,
		FVector(-0.850651, 0.000000, -0.525731) * UVerticesRenderer::MeshScale,
		FVector(-0.850651, 0.000000, 0.525731) * UVerticesRenderer::MeshScale,
		FVector(-0.500000, 0.309017, 0.809017) * UVerticesRenderer::MeshScale,
		FVector(-0.809017, 0.500000, 0.309017) * UVerticesRenderer::MeshScale,
		FVector(-0.309017, 0.809017, 0.500000) * UVerticesRenderer::MeshScale,
		FVector(0.309017, 0.809017, 0.500000) * UVerticesRenderer::MeshScale,
		FVector(0.000000, 1.000000, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(0.309017, 0.809017, -0.500000) * UVerticesRenderer::MeshScale,
		FVector(-0.309017, 0.809017, -0.500000) * UVerticesRenderer::MeshScale,
		FVector(-0.500000, 0.309017, -0.809017) * UVerticesRenderer::MeshScale,
		FVector(-0.809017, 0.500000, -0.309017) * UVerticesRenderer::MeshScale,
		FVector(-1.000000, 0.000000, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(-0.809017, -0.500000, -0.309017) * UVerticesRenderer::MeshScale,
		FVector(-0.809017, -0.500000, 0.309017) * UVerticesRenderer::MeshScale,
		FVector(-0.500000, -0.309017, 0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.000000, 0.000000, 1.000000) * UVerticesRenderer::MeshScale,
		FVector(0.500000, 0.309017, 0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.809017, 0.500000, 0.309017) * UVerticesRenderer::MeshScale,
		FVector(0.809017, 0.500000, -0.309017) * UVerticesRenderer::MeshScale,
		FVector(0.500000, 0.309017, -0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.000000, 0.000000, -1.000000) * UVerticesRenderer::MeshScale,
		FVector(-0.500000, -0.309017, -0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.500000, -0.309017, 0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.809017, -0.500000, 0.309017) * UVerticesRenderer::MeshScale,
		FVector(0.309017, -0.809017, 0.500000) * UVerticesRenderer::MeshScale,
		FVector(-0.309017, -0.809017, 0.500000) * UVerticesRenderer::MeshScale,
		FVector(0.000000, -1.000000, 0.000000) * UVerticesRenderer::MeshScale,
		FVector(-0.309017, -0.809017, -0.500000) * UVerticesRenderer::MeshScale,
		FVector(0.309017, -0.809017, -0.500000) * UVerticesRenderer::MeshScale,
		FVector(0.500000, -0.309017, -0.809017) * UVerticesRenderer::MeshScale,
		FVector(0.809017, -0.500000, -0.309017) * UVerticesRenderer::MeshScale,
		FVector(1.000000, 0.000000, 0.000000) * UVerticesRenderer::MeshScale,
	},
	{
		5, 12, 14, 11, 13, 12, 0, 14, 13, 12, 13, 14, 1, 15, 16, 5, 14, 15,
		0, 16, 14, 15, 14, 16, 7, 17, 18, 1, 16, 17, 0, 18, 16, 17, 16, 18,
		10, 19, 20, 7, 18, 19, 0, 20, 18, 19, 18, 20, 11, 21, 13, 10, 20, 21,
		0, 13, 20, 21, 20, 13, 2, 22, 23, 10, 21, 22, 11, 23, 21, 22, 21, 23,
		4, 24, 25, 11, 12, 24, 5, 25, 12, 24, 12, 25, 9, 26, 27, 5, 15, 26,
		1, 27, 15, 26, 15, 27, 8, 28, 29, 1, 17, 28, 7, 29, 17, 28, 17, 29,
		6, 30, 31, 7, 19, 30, 10, 31, 19, 30, 19, 31, 4, 32, 34, 9, 33, 32,
		3, 34, 33, 32, 33, 34, 2, 35, 36, 4, 34, 35, 3, 36, 34, 35, 34, 36,
		6, 37, 38, 2, 36, 37, 3, 38, 36, 37, 36, 38, 8, 39, 40, 6, 38, 39,
		3, 40, 38, 39, 38, 40, 9, 41, 33, 8, 40, 41, 3, 33, 40, 41, 40, 33,
		1, 28, 27, 8, 41, 28, 9, 27, 41, 28, 41, 27, 5, 26, 25, 9, 32, 26,
		4, 25, 32, 26, 32, 25, 11, 24, 23, 4, 35, 24, 2, 23, 35, 24, 35, 23,
		10, 22, 31, 2, 37, 22, 6, 31, 37, 22, 37, 31, 7, 30, 29, 6, 39, 30,
		8, 29, 39, 30, 39, 29,
	}
};

/**
 * Hard-coded icosahedron of 0 order scaled by MeshScale.
 * Used for vertex collision.
 * 
 * @link https://observablehq.com/@mourner/fast-icosphere-mesh
 */
static std::pair<
	TArray<FVector, TFixedAllocator<IcosahedronZeroVertNum>>,
	TArray<int32, TFixedAllocator<IcosahedronZeroIndNum>>
> IcosahedronCollision = {
	{
		FVector(-0.525731087f, 0.850650787f, 0.0f) * UVerticesRenderer::MeshScale,
		FVector(0.525731087f, 0.850650787f, 0.0f) * UVerticesRenderer::MeshScale,
		FVector(-0.525731087f, -0.850650787f, 0.0f) * UVerticesRenderer::MeshScale,
		FVector(0.525731087f, -0.850650787f, 0.0f) * UVerticesRenderer::MeshScale,
		FVector(0.0f, -0.525731087f, 0.850650787f) * UVerticesRenderer::MeshScale,
		FVector(0.0f, 0.525731087f, 0.850650787f) * UVerticesRenderer::MeshScale,
		FVector(0.0f, -0.525731087f, -0.850650787f) * UVerticesRenderer::MeshScale,
		FVector(0.0f, 0.525731087f, -0.850650787f) * UVerticesRenderer::MeshScale,
		FVector(0.850650787f, 0.0f, -0.525731087f) * UVerticesRenderer::MeshScale,
		FVector(0.850650787f, 0.0f, 0.525731087f) * UVerticesRenderer::MeshScale,
		FVector(-0.850650787f, 0.0f, -0.525731087f) * UVerticesRenderer::MeshScale,
		FVector(-0.850650787f, 0.0f, 0.525731087f) * UVerticesRenderer::MeshScale,
	},
	{
		5, 11, 0, 1, 5, 0, 7, 1, 0, 10, 7, 0, 11, 10, 0, 2, 10, 11,
		4, 11, 5, 9, 5, 1, 8, 1, 7, 6, 7, 10, 4, 9, 3, 2, 4, 3,
		6, 2, 3, 8, 6, 3, 9, 8, 3, 1, 8, 9, 5, 9, 4, 11, 4, 2,
		10, 2, 6, 7, 6, 8
	}
};

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

	const size_t VerticesNum = IcosahedronMesh.first.Num() * TmpData.Positions.Num();

	// mesh vertices
	{
		TArray<FVector> Positions;
		Positions.Reserve(VerticesNum);

		for (const auto &VertexPos : TmpData.Positions) {
			auto VertexMeshPositions = IcosahedronMesh.first;
			for (auto &VertexMeshPos : VertexMeshPositions)
				VertexMeshPos += VertexPos;
			Positions.Append(MoveTemp(VertexMeshPositions));
		}

		check(Positions.Num() == VerticesNum);
		MeshData.Positions.Append(Positions);
	}

	// mesh colors
	{
		TArray<FColor> Colors;
		Colors.Reserve(VerticesNum);

		for (const auto &VertexColor : TmpData.Colors) {
			TArray<FColor, TFixedAllocator<IcosahedronFirstVertNum>> VertexMeshColors;
			VertexMeshColors.Init(VertexColor, IcosahedronMesh.first.Num());
			Colors.Append(MoveTemp(VertexMeshColors));
		}

		check(Colors.Num() == VerticesNum);
		MeshData.Colors.Append(Colors);
	}

	// mesh indices
	{
		const size_t IndicesNum = IcosahedronMesh.second.Num() * TmpData.Positions.Num();
		TArray<int32> Indices;
		Indices.Reserve(IndicesNum);

		for (size_t VerticesOffset = 0; VerticesOffset < VerticesNum; VerticesOffset += IcosahedronMesh.first.Num()) {
			auto VertexMeshIndices = IcosahedronMesh.second;
			if (VerticesOffset > 0)
				for (auto &Idx : VertexMeshIndices)
					Idx += VerticesOffset;
			Indices.Append(MoveTemp(VertexMeshIndices));
		}

		check(Indices.Num() == IndicesNum);
		MeshData.Triangles.Append(Indices);
	}

	MeshData.Tangents.SetNum(VerticesNum);
	MeshData.TexCoords.SetNum(VerticesNum);

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

	// collision vertices
	{
		const auto VertexCollisionPositions = IcosahedronCollision.first;

		const size_t VerticesNum = VertexCollisionPositions.Num() * Data.Positions.Num();
		CollisionData.Vertices.Reserve(VerticesNum);

		for (const auto &VertexPos : Data.Positions) {
			for (const auto &VertexCollisionPos : VertexCollisionPositions)
				CollisionData.Vertices.Add(VertexCollisionPos + VertexPos);
		}

		check(CollisionData.Vertices.Num() == VerticesNum);
	}

	// collision triangles and sources
	{
		const auto VertexCollisionIndices = IcosahedronCollision.second;

		const size_t TrianglesNum = VertexCollisionIndices.Num() / 3 * Data.Positions.Num();
		const size_t SourcesNum = Data.Positions.Num();
		CollisionData.Triangles.Reserve(TrianglesNum);
		CollisionData.CollisionSources.Reserve(SourcesNum);

		size_t VerticesOffset = 0;
		for (const auto VertexId : Data.StorageIds) {
			const size_t StartTriangles = CollisionData.Triangles.Num();
			for (size_t j = 0; j < VertexCollisionIndices.Num(); j += 3) {
				CollisionData.Triangles.Add(
					VertexCollisionIndices[j + 0] + VerticesOffset,
					VertexCollisionIndices[j + 1] + VerticesOffset,
					VertexCollisionIndices[j + 2] + VerticesOffset
				);
			}
			const size_t EndTriangles = CollisionData.Triangles.Num() - 1;
			VerticesOffset += IcosahedronCollision.first.Num();

			CollisionData.CollisionSources.Emplace(
				StartTriangles, EndTriangles,
				this, VertexId,
				ERuntimeMeshCollisionFaceSourceType::Collision
			);
		}

		check(CollisionData.Triangles.Num() == TrianglesNum);
		check(CollisionData.CollisionSources.Num() == SourcesNum);
	}

	return true;
}
