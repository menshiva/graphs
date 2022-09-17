#pragma once

#include <array>
#include <vector>

namespace VertexMeshFactory {
	// simple constexpr pow function that returns 4^Exp
    constexpr static uint32_t Pow4(const uint32_t Exp) {
        uint32_t Res = 1;
        for (uint32_t i = 0; i < Exp; ++i)
            Res *= 4;
        return Res;
    }

    // simple constexpr sqrtf function: Newton Raphson algorithm
    constexpr static float Sqrt(const float Value) {
        float Curr = Value, Prev = 0.0f;
        while (Curr != Prev) {
            Prev = Curr;
            Curr = 0.5f * (Curr + Value / Curr);
        }
        return Curr;
    }

    struct Vec {
        constexpr Vec() : X(0.0f), Y(0.0f), Z(0.0f) {}

        constexpr Vec(const float X, const float Y, const float Z) : X(X), Y(Y), Z(Z) {}

        // note: only used for tests
        // write your own version or use another vector structure to compare components with some additional tolerance
        constexpr bool operator==(const Vec &OtherVec) const {
            return X == OtherVec.X && Y == OtherVec.Y && Z == OtherVec.Z;
        }

        // note: only used for tests
        // write your own version or use another vector structure to compare components with some additional tolerance
        constexpr bool operator!=(const Vec &OtherVec) const {
            return !(*this == OtherVec);
        }

        constexpr Vec operator+(const Vec &OtherVec) const {
            return {X + OtherVec.X, Y + OtherVec.Y, Z + OtherVec.Z};
        }

        constexpr Vec operator*(const float Value) const {
            return {X * Value, Y * Value, Z * Value};
        }

        constexpr Vec operator/(const float Value) const {
            return {X / Value, Y / Value, Z / Value};
        }

        // simple constexpr vector normalization
        constexpr void Normalize() {
            constexpr float Tolerance = 1.e-8f;
            const float SquareSize = X * X + Y * Y + Z * Z;
            if (SquareSize > Tolerance) {
                const float Scale = 1.0f / Sqrt(SquareSize);
                X *= Scale, Y *= Scale, Z *= Scale;
            } else {
                X = 0.0f, Y = 0.0f, Z = 0.0f;
            }
        }

        float X, Y, Z;
    };

    constexpr static uint32_t GetVerticesNum(const uint32_t Quality) {
        return 10 * Pow4(Quality) + 2;
    }

    constexpr static uint32_t GetIndicesNum(const uint32_t Quality) {
        return 5 * Pow4(Quality + 1) * 3;
    }

    template <uint32_t Quality>
    struct SphereMesh {
        std::array<Vec, GetVerticesNum(Quality)> Vertices;
        std::array<int32, GetIndicesNum(Quality)> Indices;
    };

    template <uint32_t Quality>
    constexpr SphereMesh<Quality> GenerateUnit() {
        constexpr auto PrevSphere = GenerateUnit<Quality - 1>();
        SphereMesh<Quality> NewSphere;

        int32 NewVerticesSize = PrevSphere.Vertices.size();

        for (int32 i = 0; i < NewVerticesSize; ++i)
            NewSphere.Vertices[i] = PrevSphere.Vertices[i];

        std::vector<std::pair<uint32_t, int32>> EdgesLookup;

        auto AddMidVertex = [&] (const uint32_t A, const uint32_t B) -> int32 {
            // Cantor's pairing function
            const uint32_t Key = (A + B) * (A + B + 1) / 2 + std::min(A, B);

            const auto Found = std::lower_bound(
                    EdgesLookup.cbegin(), EdgesLookup.cend(), Key,
                    [] (const std::pair<uint32_t, size_t> &P, const uint32_t Value) {
                        return P.first < Value;
                    }
            );
            if (Found != EdgesLookup.cend() && Found->first == Key)
                return Found->second;
            EdgesLookup.emplace(Found, Key, NewVerticesSize);

            auto &MidVertex = NewSphere.Vertices[NewVerticesSize];
            MidVertex = (NewSphere.Vertices[A] + NewSphere.Vertices[B]) / 2.0f;
            MidVertex.Normalize();

            return NewVerticesSize++;
        };

        for (size_t k = 0; k < PrevSphere.Indices.size(); k += 3) {
            // subdivide each triangle into 4 triangles
            const auto v1 = PrevSphere.Indices[k + 0];
            const auto v2 = PrevSphere.Indices[k + 1];
            const auto v3 = PrevSphere.Indices[k + 2];

            const auto a = AddMidVertex(v1, v2);
            const auto b = AddMidVertex(v2, v3);
            const auto c = AddMidVertex(v3, v1);

            auto t = k * 4;
            NewSphere.Indices[t++] = v1; NewSphere.Indices[t++] = a; NewSphere.Indices[t++] = c;
            NewSphere.Indices[t++] = v2; NewSphere.Indices[t++] = b; NewSphere.Indices[t++] = a;
            NewSphere.Indices[t++] = v3; NewSphere.Indices[t++] = c; NewSphere.Indices[t++] = b;
            NewSphere.Indices[t++] = a;  NewSphere.Indices[t++] = b; NewSphere.Indices[t] = c;
        }

        return NewSphere;
    }

    // hard-coded unit icosahedron of 0 quality
    template <>
    constexpr SphereMesh<0> GenerateUnit<0>() {
        return {{
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
        }, {
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
        }};
    }
}

namespace EdgeMeshFactory {
	constexpr static uint32_t GetVerticesNum(const uint32_t Quality) {
		return Quality * 2;
	}

	constexpr static uint32_t GetIndicesNum(const uint32_t Quality) {
		return Quality * 2 * 3;
	}

	static void GenerateMesh(
		FVector FirstVertexPos, FVector SecondVertexPos,
		const FColor &FirstVertexColor, const FColor &SecondVertexColor,
		TArray<FVector> &OutVertices, TArray<int32_t> &OutTriangles, TArray<FColor> &OutColors
	) {
		// do not generate edge if vertices have intersection
		// TODO
		/*if (FVector::DistSquared(FirstVertexPos, SecondVertexPos) <= 4 * VertexMeshFactory::SCALE * VertexMeshFactory::SCALE)
			return;

		const auto ForwardDir = (SecondVertexPos - FirstVertexPos).GetSafeNormal();

		// generate unit vector that is perpendicular to ForwardDir
		// this new vector is going to be a right directional vector
		const auto du = FVector::DotProduct(ForwardDir, FVector::UpVector);
		const auto df = FVector::DotProduct(ForwardDir, FVector::ForwardVector);
		const auto v1 = fabsf(du) < fabsf(df) ? FVector::UpVector : FVector::ForwardVector;
		auto RightDir = FVector::CrossProduct(v1, ForwardDir);
		check(FVector::Orthogonal(ForwardDir, RightDir));

		TStaticArray<FVector, QUALITY> Face;
		for (auto &V : Face) {
			V = RightDir * SCALE;
			RightDir = RightDir.RotateAngleAxis(STEP_ANGLE, ForwardDir);
		}

		// offset vertex positions almost by vertex mesh scale
		const auto Offset = ForwardDir * VertexMeshFactory::SCALE * 0.7f;
		FirstVertexPos += Offset;
		SecondVertexPos -= Offset;

		const auto VerticesOffset = OutVertices.Num();
		check(OutVertices.Num() + MESH_VERTICES_NUM <= OutVertices.Max());
		check(OutColors.Num() + MESH_VERTICES_NUM <= OutColors.Max());
		for (const auto &V : Face) {
			OutVertices.Push(FirstVertexPos + V);
			OutColors.Push(FirstVertexColor);
		}
		for (const auto &V : Face) {
			OutVertices.Push(SecondVertexPos + V);
			OutColors.Push(SecondVertexColor);
		}

		check(OutTriangles.Num() + MESH_TRIANGLES_INDICES_NUM <= OutTriangles.Max());
		for (int32 i = 0; i < QUALITY; ++i) {
			const int32 FirstFaceI = i;
			const int32 FirstFaceNextI = (i + 1) % QUALITY;
			const int32 SecondFaceI = FirstFaceI + QUALITY;
			const int32 SecondFaceNextI = FirstFaceNextI + QUALITY;
			OutTriangles.Append({
				VerticesOffset + FirstFaceI,
				VerticesOffset + SecondFaceI,
				VerticesOffset + FirstFaceNextI
			});
			OutTriangles.Append({
				VerticesOffset + SecondFaceI,
				VerticesOffset + SecondFaceNextI,
				VerticesOffset + FirstFaceNextI
			});
		}*/
	}
}
