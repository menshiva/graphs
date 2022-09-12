#include "GraphRenderer.h"
#include "ProceduralMeshComponent.h"
#include "MeshFactory.h"

enum Section : int32 {
	VERTICES = 0,
	EDGES
};

AGraphRenderer::AGraphRenderer() {
	PrimaryActorTick.bCanEverTick = false;

	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProcMesh");
	ProcMesh->PrimaryComponentTick.bCanEverTick = false;
	ProcMesh->SetMobility(EComponentMobility::Static);
	ProcMesh->SetEnableGravity(false);
	ProcMesh->CanCharacterStepUpOn = ECB_No;
	ProcMesh->SetCollisionProfileName("Graph");
	ProcMesh->SetCastShadow(false);
	ProcMesh->bUseAsyncCooking = true;

	ProcMesh->CreateMeshSection_LinearColor(
		VERTICES,
		TArray<FVector>(),
		TArray<int32>(),
		TArray<FVector>(),
		TArray<FVector2D>(),
		TArray<FLinearColor>(),
		TArray<FProcMeshTangent>(),
		true
	);
	ProcMesh->SetMaterial(
		VERTICES,
		ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object
	);
	ProcMesh->CreateMeshSection_LinearColor(
		EDGES,
		TArray<FVector>(),
		TArray<int32>(),
		TArray<FVector>(),
		TArray<FVector2D>(),
		TArray<FLinearColor>(),
		TArray<FProcMeshTangent>(),
		true
	);
	ProcMesh->SetMaterial(
		EDGES,
		ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object
	);

	RootComponent = ProcMesh;
}

EntityId AGraphRenderer::GetEntityIdFromCollisionData(const int32 FaceIndex) const {
	if (FaceIndex >= 0 && FaceIndex < CollisionData.Num())
		return CollisionData[FaceIndex];
	return EntityId::NONE();
}

bool AGraphRenderer::ExecuteCommand(Command&& Cmd, const bool MarkDirty) {
	const auto CommandResult = Cmd.Implementation(Storage);
	if (CommandResult && MarkDirty)
		this->MarkDirty();
	return CommandResult;
}

void AGraphRenderer::MarkDirty() {
	TArray<FVector> Vertices;
	TArray<int32_t> Triangles;
	TArray<FColor> Colors;

	const auto &VerticesStorage = Storage.GetStorage<VertexEntity>().Data;
	const auto &EdgesStorage = Storage.GetStorage<EdgeEntity>().Data;

	if (VerticesStorage.Num() == 0) {
		check(Storage.GetStorage<GraphEntity>().Data.Num() == 0);
		check(Storage.GetStorage<EdgeEntity>().Data.Num() == 0);
	}

	const size_t VerticesVerticesNum = VertexMeshFactory::MESH_VERTICES_NUM * VerticesStorage.Num();
	const size_t VerticesTrianglesNum = VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM * VerticesStorage.Num();

	const size_t EdgesVerticesNum = EdgeMeshFactory::MESH_VERTICES_NUM * EdgesStorage.Num();
	const size_t EdgesTrianglesNum = EdgeMeshFactory::MESH_TRIANGLES_INDICES_NUM * EdgesStorage.Num();

	check(VerticesTrianglesNum % 3 == 0);
	check(EdgesTrianglesNum % 3 == 0);
	const size_t CollisionDataNum = (VerticesTrianglesNum + EdgesTrianglesNum) / 3;
	CollisionData.Reset(CollisionDataNum); // TODO: update collision data only when calling CreateMeshSection
	TArray<EntityId> EntityCollisionData;

	// Vertices
	{
		Vertices.Reserve(VerticesVerticesNum);
		Triangles.Reserve(VerticesTrianglesNum);
		Colors.Reserve(VerticesVerticesNum);

		for (auto VertexEntIter = VerticesStorage.CreateConstIterator(); VertexEntIter; ++VertexEntIter) {
			const auto &Color = VertexEntIter->IsHit
				? ColorConsts::BlueColor
				: VertexEntIter->OverrideColor != ColorConsts::OverrideColorNone
					? VertexEntIter->OverrideColor
					: VertexEntIter->Color;
			VertexMeshFactory::GenerateMesh(
				VertexEntIter->Position, Color,
				Vertices, Triangles, Colors
			);

			EntityCollisionData.Init(
				EntityId(VertexEntIter.GetIndex(), EntitySignature::VERTEX),
				VertexMeshFactory::MESH_TRIANGLES_INDICES_NUM / 3
			);
			CollisionData.Append(EntityCollisionData);
		}

		check(Vertices.Num() == VerticesVerticesNum);
		check(Triangles.Num() == VerticesTrianglesNum);
		check(Colors.Num() == VerticesVerticesNum);
	}
	UpdateSection(VERTICES, Vertices, Triangles, Colors);

	// Edges
	if (EdgesStorage.Num() > 0) {
		Vertices.Reset(EdgesVerticesNum);
		Triangles.Reset(EdgesTrianglesNum);
		Colors.Reset(EdgesVerticesNum);

		for (auto EdgeEntIter = EdgesStorage.CreateConstIterator(); EdgeEntIter; ++EdgeEntIter) {
			const auto &FromVertEnt = Storage.GetEntity<VertexEntity>(EdgeEntIter->VerticesIds[0]);
			const auto &ToVertEnt = Storage.GetEntity<VertexEntity>(EdgeEntIter->VerticesIds[1]);

			FLinearColor FromVertColor, ToVertColor;
			if (EdgeEntIter->IsHit) {
				FromVertColor = ColorConsts::BlueColor;
				ToVertColor = ColorConsts::BlueColor;
			}
			else if (EdgeEntIter->OverrideColor != ColorConsts::OverrideColorNone) {
				FromVertColor = EdgeEntIter->OverrideColor;
				ToVertColor = EdgeEntIter->OverrideColor;
			}
			else {
				FromVertColor = FromVertEnt.Color;
				ToVertColor = ToVertEnt.Color;
			}

			EdgeMeshFactory::GenerateMesh(
				FromVertEnt.Position, ToVertEnt.Position,
				FromVertColor, ToVertColor,
				Vertices, Triangles, Colors
			);

			EntityCollisionData.Init(
				EntityId(EdgeEntIter.GetIndex(), EntitySignature::EDGE),
				EdgeMeshFactory::MESH_TRIANGLES_INDICES_NUM / 3
			);
			CollisionData.Append(EntityCollisionData);
		}

		check(Vertices.Num() <= EdgesVerticesNum);
		check(Triangles.Num() <= EdgesTrianglesNum);
		check(Colors.Num() <= EdgesVerticesNum);
	}
	UpdateSection(EDGES, Vertices, Triangles, Colors);
}

void AGraphRenderer::UpdateSection(
	const int32 SectionIdx,
	const TArray<FVector> &Vertices,
	const TArray<int32_t> &Triangles,
	const TArray<FColor> &Colors
) const {
	if (ProcMesh->GetProcMeshSection(SectionIdx)->ProcIndexBuffer.Num() == Triangles.Num()) {
		ProcMesh->UpdateMeshSection(
			SectionIdx,
			Vertices,
			TArray<FVector>(),
			TArray<FVector2D>(),
			Colors,
			TArray<FProcMeshTangent>()
		);
	}
	else {
		ProcMesh->CreateMeshSection(
			SectionIdx,
			Vertices,
			Triangles,
			TArray<FVector>(),
			TArray<FVector2D>(),
			Colors,
			TArray<FProcMeshTangent>(),
			true
		);
	}
}
