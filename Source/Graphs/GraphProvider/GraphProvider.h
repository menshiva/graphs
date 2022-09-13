#pragma once

#include "Entities/Entity.h"
#include "Graphs/Utils/Consts.h"
#include "GraphProvider.generated.h"

DECLARE_CYCLE_STAT(TEXT("AGraphProvider::IsEntityValid"), STAT_AGraphProvider_IsEntityValid, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("AGraphProvider::GetEntityType"), STAT_AGraphProvider_GetEntityType, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("AGraphProvider::ExecuteCommand"), STAT_AGraphProvider_ExecuteCommand, STATGROUP_GRAPHS_PERF);

DECLARE_CYCLE_STAT(TEXT("AGraphProvider::Command::CreateEntity"), STAT_AGraphProvider_Command_CreateEntity, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("AGraphProvider::Command::GetEntity"), STAT_AGraphProvider_Command_GetEntity, STATGROUP_GRAPHS_PERF);
DECLARE_CYCLE_STAT(TEXT("AGraphProvider::Command::RemoveEntity"), STAT_AGraphProvider_Command_RemoveEntity, STATGROUP_GRAPHS_PERF);

UCLASS()
class GRAPHS_API AGraphProvider final : public AActor {
	GENERATED_BODY()
public:
	class Command {
	public:
		explicit Command(TFunction<void(AGraphProvider &Provider)> &&Impl) : Implementation(MoveTemp(Impl)) {}
		virtual ~Command() = default;
	protected:
		template <class Ent>
		static Ent *CreateEntity(AGraphProvider &Provider) {
			SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_Command_CreateEntity);
			const auto NewEntity = new Ent(Provider);
			Provider.Entities.Emplace(NewEntity);
			NewEntity->Actor->AttachToActor(&Provider, FAttachmentTransformRules::KeepRelativeTransform);
			Provider.Actors.Add(NewEntity->Actor.Get());
			return NewEntity;
		}

		template <class Ent>
		FORCEINLINE static Ent *GetEntity(const AGraphProvider &Provider, const EntityId Id) {
			SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_Command_GetEntity);
			return dynamic_cast<Ent*>(Provider.Entities.Find(Id)->Get());
		}

		static void RemoveEntity(AGraphProvider &Provider, const EntityId Id) {
			SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_Command_RemoveEntity);
			const auto EntityActor = GetEntity<Entity>(Provider, Id)->Actor.Get();
			Provider.Entities.Remove(Id);
			Provider.Actors.Remove(EntityActor);
			Provider.GetWorld()->DestroyActor(EntityActor);
		}
	private:
		TFunction<void(AGraphProvider &Provider)> Implementation;
		friend AGraphProvider;
	};

	AGraphProvider() {
		PrimaryActorTick.bCanEverTick = false;

		EntityMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object;
		VertexMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere")).Object;
		EdgeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Engine/BasicShapes/Cylinder")).Object;
	}

	FORCEINLINE UMaterial *GetEntityActorMaterial() const { return EntityMaterial; }
	FORCEINLINE UStaticMesh *GetVertexMesh() const { return VertexMesh; }
	FORCEINLINE UStaticMesh *GetEdgeMesh() const { return EdgeMesh; }

	FORCEINLINE bool IsEntityValid(const EntityId Id) const {
		SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_IsEntityValid);
		return Entities.Contains(Id);
	}

	FORCEINLINE EntityType GetEntityType(const EntityId Id) const {
		SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_GetEntityType);
		return Entities.Find(Id)->Get()->Type;
	}

	FORCEINLINE void ExecuteCommand(Command &&Cmd) {
		SCOPE_CYCLE_COUNTER(STAT_AGraphProvider_ExecuteCommand);
		Cmd.Implementation(*this);
	}
private:
	UPROPERTY()
	UMaterial *EntityMaterial;

	UPROPERTY()
	UStaticMesh *VertexMesh;

	UPROPERTY()
	UStaticMesh *EdgeMesh;

	UPROPERTY()
	TSet<AActor*> Actors;

	TSet<TUniquePtr<Entity>, EntityKeyFuncs> Entities;
};
