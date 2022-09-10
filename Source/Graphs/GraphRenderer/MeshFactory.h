#pragma once

#include <array>
#include <vector>

namespace VertexMeshFactory {
	constexpr static uint32_t QUALITY = 1;
	constexpr static float SCALE = 50.0f;

	// simple constexpr version of FVector
	struct Vec {
		constexpr Vec() : X(0.0f), Y(0.0f), Z(0.0f) {}
		constexpr Vec(const float X, const float Y, const float Z) : X(X), Y(Y), Z(Z) {}

		float X, Y, Z;
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

	constexpr static uint32_t GetMeshTrianglesIndicesNum(const uint32_t Quality) {
		return 5 * Pow4(Quality + 1) * 3;
	}

	template <uint32_t Quality>
	constexpr static std::pair<
		std::array<Vec, GetMeshVerticesNum(Quality)>,
		std::array<int32, GetMeshTrianglesIndicesNum(Quality)>
	> GenerateIcosahedron() {
	    constexpr auto PrevQualityIcosahedron = GenerateIcosahedron<Quality - 1>();
	    auto &PrevVertices = PrevQualityIcosahedron.first;
	    auto &PrevTriangles = PrevQualityIcosahedron.second;

	    std::array<Vec, GetMeshVerticesNum(Quality)> NewVertices;
	    std::array<int32, GetMeshTrianglesIndicesNum(Quality)> NewTriangles;

    	for (size_t i = 0; i < PrevVertices.size(); ++i)
    	    NewVertices[i] = PrevVertices[i];

    	size_t NewVerticesSize = PrevVertices.size();
    	std::vector<std::pair<int32, int32>> Lookup;

		auto AddMidPoint = [&] (const int32 A, const int32 B) -> int32 {
			// Cantor's pairing function
			const int32 Key = (A + B) * (A + B + 1) / 2 + std::min(A, B);

			for (size_t k = 0; k < Lookup.size(); ++k) {
				if (Lookup[k].first == Key) {
					const auto Ret = Lookup[k].second;
					Lookup.erase(Lookup.begin() + k);
					return Ret;
				}
			}

			Lookup.emplace_back(Key, NewVerticesSize);

			auto &MidPoint = NewVertices[NewVerticesSize];
			MidPoint.X = (NewVertices[A].X + NewVertices[B].X) / 2.0f;
			MidPoint.Y = (NewVertices[A].Y + NewVertices[B].Y) / 2.0f;
			MidPoint.Z = (NewVertices[A].Z + NewVertices[B].Z) / 2.0f;

			// normalize mid point
			const float SquareSum =
				  MidPoint.X * MidPoint.X
				+ MidPoint.Y * MidPoint.Y
				+ MidPoint.Z * MidPoint.Z;
			float Curr = SquareSum, Prev = 0.0f;
			while (Curr != Prev) {
				// simple constexpr version of sqruare root: Newton Raphson algorithm
				Prev = Curr;
				Curr = 0.5f * (Curr + SquareSum / Curr);
			}
			const float Scale = 1.0f / Curr;
			MidPoint.X *= Scale;
			MidPoint.Y *= Scale;
			MidPoint.Z *= Scale;

			return NewVerticesSize++;
		};

		for (size_t k = 0; k < PrevTriangles.size(); k += 3) {
			// subdivide each triangle into 4 triangles
			const auto v1 = PrevTriangles[k + 0];
			const auto v2 = PrevTriangles[k + 1];
			const auto v3 = PrevTriangles[k + 2];
			const auto a = AddMidPoint(v1, v2);
			const auto b = AddMidPoint(v2, v3);
			const auto c = AddMidPoint(v3, v1);

			auto t = k * 4;
			NewTriangles[t++] = v1; NewTriangles[t++] = a; NewTriangles[t++] = c;
			NewTriangles[t++] = v2; NewTriangles[t++] = b; NewTriangles[t++] = a;
			NewTriangles[t++] = v3; NewTriangles[t++] = c; NewTriangles[t++] = b;
			NewTriangles[t++] = a;  NewTriangles[t++] = b; NewTriangles[t] = c;
		}

		return {NewVertices, NewTriangles};
	}

	template <>
	constexpr std::pair<
		std::array<Vec, GetMeshVerticesNum(0)>,
		std::array<int32, GetMeshTrianglesIndicesNum(0)>
	> GenerateIcosahedron<0>() {
		return {
		    {
		    	Vec(-0.525731087f, 0.850650787f, 0.0f),
		    	Vec(0.525731087f, 0.850650787f, 0.0f),
		    	Vec(-0.525731087f, -0.850650787f, 0.0f),
		    	Vec(0.525731087f, -0.850650787f, 0.0f), 
				Vec(0.0f, -0.525731087f, 0.850650787f),
		    	Vec(0.0f, 0.525731087f, 0.850650787f),
		    	Vec(0.0f, -0.525731087f, -0.850650787f),
		    	Vec(0.0f, 0.525731087f, -0.850650787f), 
				Vec(0.850650787f, 0.0f, -0.525731087f),
		    	Vec(0.850650787f, 0.0f, 0.525731087f),
		    	Vec(-0.850650787f, 0.0f, -0.525731087f),
		    	Vec(-0.850650787f, 0.0f, 0.525731087f)
			},
			{
				5, 11, 0,
				1, 5, 0,
				7, 1, 0,
				10, 7, 0,
				11, 10, 0,
				2, 10, 11,
				4, 11, 5,
				9, 5, 1,
				8, 1, 7,
				6, 7, 10,
				4, 9, 3,
				2, 4, 3,
				6, 2, 3,
				8, 6, 3,
				9, 8, 3,
				1, 8, 9,
				5, 9, 4,
				11, 4, 2,
				10, 2, 6,
				7, 6, 8
			}
		};
	}

	constexpr static size_t MESH_VERTICES_NUM = GetMeshVerticesNum(QUALITY);
	constexpr static size_t MESH_TRIANGLES_INDICES_NUM = GetMeshTrianglesIndicesNum(QUALITY);

	static void GenerateMesh(
		const FVector &Origin, const FLinearColor &Color,
		TArray<FVector> &OutVertices, TArray<int32_t> &OutTriangles, TArray<FLinearColor> &OutColors
	) {
		constexpr static auto Icosahedron = GenerateIcosahedron<QUALITY>();
		static_assert(Icosahedron.first[0].X == -0.525731087f);
		static_assert(Icosahedron.first[0].Y == 0.850650787f);
		static_assert(Icosahedron.first[0].Z == 0.0f);

		const auto VerticesOffset = OutVertices.Num();

		check(OutVertices.Num() + Icosahedron.first.size() <= OutVertices.Max());
		check(OutColors.Num() + Icosahedron.first.size() <= OutColors.Max());
		for (const auto &V : Icosahedron.first) {
			OutVertices.Push(Origin + *reinterpret_cast<const FVector*>(&V) * SCALE);
			OutColors.Push(Color);
		}

		check(OutTriangles.Num() + Icosahedron.second.size() <= OutTriangles.Max());
		for (const auto T : Icosahedron.second)
			OutTriangles.Push(VerticesOffset + T);
	}
}
