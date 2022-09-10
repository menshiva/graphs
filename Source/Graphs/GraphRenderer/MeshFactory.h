#pragma once

#include <array>
#include <vector>

namespace VertexMeshFactory {
	constexpr static uint32_t QUALITY = 1;
	constexpr static float SIZE = 50.0f;

	// simple constexpr version of FVector
	struct Vec {
		constexpr Vec() : X(0.0f), Y(0.0f), Z(0.0f) {}
		constexpr Vec(const float X, const float Y, const float Z) : X(X), Y(Y), Z(Z) {}

		constexpr static float SqrtNewtonRaphson(const float Val, const float Curr, const float Prev) {
			// TODO: maybe remove
			return Curr == Prev
				? Curr
				: SqrtNewtonRaphson(Val, 0.5f * (Curr + Val / Curr), Curr);
		}

		constexpr void Normalize() {
			// TODO: maybe remove
			constexpr float Tolerance = 1e-08;
			const float SquareSum = X * X + Y * Y + Z * Z;
			if (SquareSum > Tolerance) {
				const float Scale = 1.0f / SqrtNewtonRaphson(SquareSum, SquareSum, 0.0f);
				X *= Scale; Y *= Scale; Z *= Scale;
			}
			else {
				X = 0.0f; Y = 0.0f; Z = 0.0f;
			}
		}

		float X, Y, Z;
	};

	struct Triangle {
		constexpr Triangle() : VertsIdx() {}
		constexpr Triangle(const int32 A, const int32 B, const int32 C) : VertsIdx({A, B, C}) {}
		std::array<int32, 3> VertsIdx;
	};

	// simple constexpr version of pow function that returns 4^Exp
	constexpr static uint32_t Pow4(const uint32_t Exp) {
		uint32_t Res = 1;
		for (uint32_t i = 0; i < Exp; ++i)
			Res *= 4;
		return Res;
	}

	constexpr static uint32_t GetMeshVerticesNum(const uint32_t Quality) {
		return 10 * Pow4(Quality) + 2;
	}

	constexpr static uint32_t GetMeshTrianglesNum(const uint32_t Quality) {
		return 5 * Pow4(Quality + 1);
	}

	template <uint32_t Quality>
	constexpr static std::pair<
		std::array<Vec, GetMeshVerticesNum(Quality)>,
		std::array<Triangle, GetMeshTrianglesNum(Quality)>
	> GenerateIcosahedron() {
	    constexpr auto PrevQualityIcosahedron = GenerateIcosahedron<Quality - 1>();
	    auto &PrevVertices = PrevQualityIcosahedron.first;
	    auto &PrevTriangles = PrevQualityIcosahedron.second;
	    std::array<Vec, GetMeshVerticesNum(Quality)> NewVertices;
	    std::array<Triangle, GetMeshTrianglesNum(Quality)> NewTriangles;

    	for (size_t i = 0; i < PrevVertices.size(); ++i)
    	    NewVertices[i] = PrevVertices[i];
    	size_t NewVerticesSize = PrevVertices.size();

    	std::vector<std::pair<std::pair<uint32_t, uint32_t>, size_t>> Lookup;
    	for (size_t i = 0; i < PrevTriangles.size(); ++i) {
    	    const auto PrevTriangle = PrevTriangles[i];
    	    std::array<int32, 3> Mid{};

        	for (int Edge = 0; Edge < 3; ++Edge) {
        	    auto Vi1 = PrevTriangle.VertsIdx[Edge];
        	    auto Vi2 = PrevTriangle.VertsIdx[(Edge + 1) % 3];
        	    if (Vi1 > Vi2) std::swap(Vi1, Vi2);

            	bool Found = false;
            	for (size_t k = 0; k < Lookup.size(); ++k) {
            	    if (Lookup[k].first.first == Vi1 && Lookup[k].first.second == Vi2) {
            	        Mid[Edge] = Lookup[k].second;
            	        Found = true;
            	        break;
            	    }
            	}

            	if (!Found) {
            	    Mid[Edge] = NewVerticesSize;
            	    Lookup.push_back({{Vi1, Vi2}, NewVerticesSize});
            	    const auto &EdgeVert0 = NewVertices[Vi1];
            	    const auto &EdgeVert1 = NewVertices[Vi2];
            	    Vec NewEdgeVert(
            	            EdgeVert0.X + EdgeVert1.X,
            	            EdgeVert0.Y + EdgeVert1.Y,
            	            EdgeVert0.Z + EdgeVert1.Z
            	    );
            	    NewEdgeVert.Normalize(); // TODO: / 2.0f
            	    NewVertices[NewVerticesSize++] = NewEdgeVert;
            	}
			}

        	NewTriangles[i * 4 + 0] = Triangle(PrevTriangle.VertsIdx[0], Mid[0], Mid[2]);
        	NewTriangles[i * 4 + 1] = Triangle(PrevTriangle.VertsIdx[1], Mid[1], Mid[0]);
        	NewTriangles[i * 4 + 2] = Triangle(PrevTriangle.VertsIdx[2], Mid[2], Mid[1]);
        	NewTriangles[i * 4 + 3] = Triangle(Mid[0], Mid[1], Mid[2]);
		}

		return {NewVertices, NewTriangles};
	}

	template <>
	constexpr std::pair<
		std::array<Vec, GetMeshVerticesNum(0)>,
		std::array<Triangle, GetMeshTrianglesNum(0)>
	> GenerateIcosahedron<0>() {
		return {
		    {
		    	Vec(-0.525731f, 0.0f, 0.850651f),
				Vec(0.525731f, 0.0f, 0.850651f),
				Vec(-0.525731f, 0.0f, -0.850651f),
				Vec(0.525731f, 0.0f, -0.850651f),
				Vec(0.0f, 0.850651f, 0.525731f),
				Vec(0.0f, 0.850651f, -0.525731f),
				Vec(0.0f, -0.850651f, 0.525731f),
				Vec(0.0f, -0.850651f, -0.525731f),
				Vec(0.850651f, 0.525731f, 0.0f),
				Vec(-0.850651f, 0.525731f, 0.0f),
				Vec(0.850651f, -0.525731f, 0.0f),
				Vec(-0.850651f, -0.525731f, 0.0f)
			},
			{
				Triangle(0, 4, 1),
				Triangle(0, 9, 4),
				Triangle(9, 5, 4),
				Triangle(4, 5, 8),
				Triangle(4, 8, 1),
				Triangle(8, 10, 1),
				Triangle(8, 3, 10),
				Triangle(5, 3, 8),
				Triangle(5, 2, 3),
				Triangle(2, 7, 3),
				Triangle(7, 10, 3),
				Triangle(7, 6, 10),
				Triangle(7, 11, 6),
				Triangle(11, 0, 6),
				Triangle(0, 1, 6),
				Triangle(6, 1, 10),
				Triangle(9, 0, 11),
				Triangle(9, 11, 2),
				Triangle(9, 2, 5),
				Triangle(7, 2, 11)
			}
		};
	}

	constexpr static size_t MESH_VERTICES_NUM = GetMeshVerticesNum(QUALITY);
	constexpr static size_t MESH_TRIANGLES_INDICES_NUM = GetMeshTrianglesNum(QUALITY) * 3;

	static void GenerateMesh(
		const FVector &Origin, const FLinearColor &Color,
		TArray<FVector> &OutVertices, TArray<int32_t> &OutTriangles, TArray<FLinearColor> &OutColors
	) {
		constexpr static auto Icosahedron = GenerateIcosahedron<QUALITY>();
		static_assert(Icosahedron.first[0].X == -0.525731f);
		static_assert(Icosahedron.first[0].Y == 0.0f);
		static_assert(Icosahedron.first[0].Z == 0.850651f);

		const int32 VerticesOffset = OutVertices.Num();

		check(OutVertices.Max() >= OutVertices.Num() + Icosahedron.first.size());
		check(OutColors.Max() >= OutColors.Num() + Icosahedron.first.size());
		for (const auto &V : Icosahedron.first) {
			OutVertices.Push(Origin + *reinterpret_cast<const FVector*>(&V) * SIZE);
			OutColors.Push(Color);
		}

		check(OutTriangles.Max() >= OutTriangles.Num() + Icosahedron.second.size());
		for (const auto &T : Icosahedron.second) {
			OutTriangles.Append({
				VerticesOffset + T.VertsIdx[0],
				VerticesOffset + T.VertsIdx[1],
				VerticesOffset + T.VertsIdx[2]
			});
		}
	}
}
