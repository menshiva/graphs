#pragma once

#include "EntityStorage/EntityStorage.h"
#include "GraphRenderer.generated.h"

UCLASS()
class GRAPHS_API AGraphRenderer final : public AActor {
	GENERATED_BODY()
public:
	using CommandImplementation = TFunction<bool(EntityStorage &Storage)>;

	class Command {
	public:
		explicit Command(CommandImplementation &&Impl) : Implementation(MoveTemp(Impl)) {}
		virtual ~Command() = default;
	protected:
		FORCEINLINE static bool ExecuteSubCommand(Command &&Cmd, EntityStorage &Storage) {
			return Cmd.Implementation(Storage);
		}

		CommandImplementation Implementation;
		friend AGraphRenderer;
	};

	AGraphRenderer();

	FORCEINLINE const EntityStorage &GetEntityStorage() const {
		return Storage;
	}

	FORCEINLINE EntityId GetEntityIdFromCollisionData(const int32 FaceIndex) const {
		if (FaceIndex >= 0 && FaceIndex < CollisionData.Num())
			return CollisionData[FaceIndex];
		return EntityId::NONE();
	}

	FORCEINLINE void PushCommand(Command &&Cmd) {
		CommandQueue.Enqueue(MoveTemp(Cmd.Implementation));
	}

	void MarkDirty();
private:
	void UpdateSection(
		int32 SectionIdx,
		const TArray<FVector> &Vertices,
		const TArray<int32_t> &Triangles,
		const TArray<FColor> &Colors
	) const;

	UPROPERTY()
	class UProceduralMeshComponent *ProcMesh;

	EntityStorage Storage;
	TQueue<CommandImplementation> CommandQueue;

	TArray<EntityId> CollisionData;
};
