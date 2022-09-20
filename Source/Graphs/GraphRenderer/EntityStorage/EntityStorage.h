#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"

DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetValidIndices"), STAT_EntityStorage_GetValidIndices, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::IsValid"), STAT_EntityStorage_IsValid, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetEntity"), STAT_EntityStorage_GetEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::ReserveForNewEntities"), STAT_EntityStorage_ReserveForNewEntities, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetEntityMut"), STAT_EntityStorage_GetEntityMut, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::NewEntity"), STAT_EntityStorage_NewEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::RemoveEntity"), STAT_EntityStorage_RemoveEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::Clear"), STAT_EntityStorage_Clear, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetStorage"), STAT_EntityStorage_GetStorage, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetStorageMut"), STAT_EntityStorage_GetStorageMut, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);

class ES {
public:
	template <typename Entity>
	using Storage = TSparseArray<Entity>;

	ES(const ES&) = delete;
	ES(ES&&) noexcept = delete;
	ES &operator=(const ES&) = delete;
	ES &operator=(ES&&) noexcept = delete;

	template <typename Entity>
	static TArray<EntityId> GetValidIndices() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetValidIndices);
		const auto &Storage = GetStorage<Entity>();

		TArray<EntityId> Out;
		Out.Reserve(Storage.Num());
		for (int32 Idx = 0; Idx < Storage.GetMaxIndex(); ++Idx)
			if (Storage.IsValidIndex(Idx))
				Out.Emplace(Idx, Entity::Signature);
		check(Out.Num() == Storage.Num());

		return Out;
	}

	template <typename Entity>
	static bool IsValid(const EntityId Id) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_IsValid);
		if (Entity::Signature != Id.Signature || Id == EntityId::NONE())
			return false;
		return GetStorage<Entity>().IsValidIndex(Id.Index);
	}

	template <typename Entity>
	FORCEINLINE static const Entity &GetEntity(const EntityId Id) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetEntity);
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		return GetStorage<Entity>()[Id.Index];
	}

	template <typename Entity>
	void ReserveForNewEntities(size_t NumToAdd) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_ReserveForNewEntities);
		auto &Storage = GetStorageMut<Entity>();
		Storage.Reserve(Storage.Num() + NumToAdd);
	}

	template <typename Entity>
	FORCEINLINE Entity &GetEntityMut(const EntityId Id) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetEntityMut);
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		return GetStorageMut<Entity>()[Id.Index];
	}

	template <typename Entity>
	FORCEINLINE EntityId NewEntity() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_NewEntity);
		return {
			static_cast<uint32_t>(GetStorageMut<Entity>().Emplace()),
			Entity::Signature
		};
	}

	template <typename Entity>
	FORCEINLINE void RemoveEntity(const EntityId Id) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_RemoveEntity);
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.Signature);
		GetStorageMut<Entity>().RemoveAt(Id.Index);
	}

	template <typename Entity>
	FORCEINLINE void Clear() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_Clear);
		GetStorageMut<Entity>().Empty();
	}
private:
	ES() = default;

	static ES &GetInstance() {
		static ES Instance;
		return Instance;
	}

	template <typename Entity>
	FORCEINLINE static const Storage<Entity> &GetStorage() {
		return GetInstance().GetStorageMut<Entity>();
	}

	template <typename Entity>
	FORCEINLINE Storage<Entity> &GetStorageMut() {
		UE_LOG(LogTemp, Fatal, TEXT("Undefined type!"));
		return Storage<Entity>();
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE Storage<VertexEntity> &GetStorageMut<VertexEntity>() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetStorage);
		return Vertices;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE Storage<EdgeEntity> &GetStorageMut<EdgeEntity>() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetStorage);
		return Edges;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE Storage<GraphEntity> &GetStorageMut<GraphEntity>() {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_GetStorage);
		return Graphs;
	}

	Storage<VertexEntity> Vertices;
	Storage<EdgeEntity> Edges;
	Storage<GraphEntity> Graphs;

	friend class GraphsRendererCommand;
};
