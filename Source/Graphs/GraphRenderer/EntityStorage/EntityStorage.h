#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"
#include "Graphs/Utils/Utils.h"

struct IStorage {
	virtual ~IStorage() = default;
};

template <typename Entity>
struct Storage final : IStorage {
	Storage() = default;
	Storage(const Storage &) = delete;
	Storage(Storage &&) = delete;
	Storage &operator=(const Storage &) = delete;
	Storage &operator=(Storage &&) = delete;

	TSparseArray<Entity> Data;
};

class EntityStorage {
public:
	EntityStorage() {
		InitStorage<GraphEntity>();
		InitStorage<VertexEntity>();
		InitStorage<EdgeEntity>();
	}

	template <typename Entity>
	FORCEINLINE const Storage<Entity> &GetStorage() const {
		return *dynamic_cast<Storage<Entity>*>(Storages[Utils::EnumUnderlyingType(Entity::Signature)].Get());
	}

	template <typename Entity>
	bool IsValid(const EntityId &Id) const {
		if (Entity::Signature != Id.Signature || Id == EntityId::NONE())
			return false;
		if (Utils::EnumUnderlyingType(Id.Signature) >= Utils::EnumUnderlyingType(EntitySignature::SIZE))
			return false;
		return GetStorage<Entity>().Data.IsValidIndex(Id.Index);
	}

	bool IsValid(const EntityId &Id) const {
		switch (Id.Signature) {
			case EntitySignature::VERTEX:
				return IsValid<VertexEntity>(Id);
			case EntitySignature::EDGE:
				return IsValid<EdgeEntity>(Id);
			case EntitySignature::GRAPH:
				return IsValid<GraphEntity>(Id);
			default:
				return false;
		}
	}

	template <typename Entity>
	const Entity &GetEntity(const EntityId &Id) const {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		return GetStorage<Entity>().Data[Id.Index];
	}

	template <typename Entity>
	FORCEINLINE Storage<Entity> &GetStorageMut() {
		return *dynamic_cast<Storage<Entity>*>(Storages[Utils::EnumUnderlyingType(Entity::Signature)].Get());
	}

	template <typename Entity>
	Entity &GetEntityMut(const EntityId &Id) {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		return GetStorageMut<Entity>().Data[Id.Index];
	}

	template <typename Entity>
	FORCEINLINE EntityId NewEntity() {
		return {
			static_cast<uint32_t>(GetStorageMut<Entity>().Data.Emplace()),
			Entity::Signature
		};
	}

	template <typename Entity>
	void RemoveEntity(const EntityId &Id) {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		GetStorageMut<Entity>().Data.RemoveAt(Id.Index);
	}
private:
	template <typename Entity>
	FORCEINLINE void InitStorage() {
		Storages[Utils::EnumUnderlyingType(Entity::Signature)] = MakeUnique<Storage<Entity>>();
	}

	TStaticArray<TUniquePtr<IStorage>, Utils::EnumUnderlyingType(EntitySignature::SIZE)> Storages;
};
