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

	EntityId GetEntityIdFromCollisionData(const int32 FaceIndex) const;

	bool ExecuteCommand(Command &&Cmd, bool MarkDirty = false);

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

	TArray<EntityId> CollisionData;
};
