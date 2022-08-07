#pragma once

#include "Entities/Entity.h"
#include "GraphProvider.generated.h"

UCLASS()
class GRAPHS_API AGraphProvider final : public AActor {
	GENERATED_BODY()
public:
	AGraphProvider() {
		PrimaryActorTick.bCanEverTick = false;

		VertexMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Engine/BasicShapes/Sphere")).Object;
		VertexMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("/Game/Graphs/Materials/GraphMaterial")).Object;
	}

	FORCEINLINE UStaticMesh *GetVertexMesh() const { return VertexMesh; }
	FORCEINLINE UMaterial *GetVertexMaterial() const { return VertexMaterial; }

	FORCEINLINE bool IsEntityValid(const EntityId Id) const { return Entities.Contains(Id); }
	FORCEINLINE EntityType GetEntityType(const EntityId Id) const { return Entities.Find(Id)->Get()->Type; }

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

	template <class T, typename... ArgsType>
	void ExecuteCommand(ArgsType&&... Args) {
		T Cmd(Forward<ArgsType>(Args)...);
		Cmd.Implementation(*this);
	}
private:
	UPROPERTY()
	UStaticMesh *VertexMesh;

	UPROPERTY()
	UMaterial *VertexMaterial;

	UPROPERTY()
	TSet<AActor*> Actors;

	TSet<TUniquePtr<Entity>, EntityKeyFuncs> Entities;
};
