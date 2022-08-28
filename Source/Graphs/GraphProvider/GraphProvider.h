#pragma once

#include "Entities/Entity.h"
#include "GraphProvider.generated.h"

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
			const auto NewEntity = new Ent(Provider);
			Provider.Entities.Emplace(NewEntity);
			NewEntity->Actor->AttachToActor(&Provider, FAttachmentTransformRules::KeepRelativeTransform);
			Provider.Actors.Add(NewEntity->Actor.Get());
			return NewEntity;
		}

		template <class Ent>
		FORCEINLINE static Ent *GetEntity(const AGraphProvider &Provider, const EntityId Id) {
			return dynamic_cast<Ent*>(Provider.Entities.Find(Id)->Get());
		}

		static void RemoveEntity(AGraphProvider &Provider, const EntityId Id) {
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

	FORCEINLINE bool IsEntityValid(const EntityId Id) const { return Entities.Contains(Id); }
	FORCEINLINE EntityType GetEntityType(const EntityId Id) const { return Entities.Find(Id)->Get()->Type; }
	FORCEINLINE void ExecuteCommand(Command &&Cmd) { Cmd.Implementation(*this); }
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
