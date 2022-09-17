#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"

DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetStorage"), STAT_EntityStorage_GetStorage, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::IsValidTemplate"), STAT_EntityStorage_IsValidTemplate, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetEntity"), STAT_EntityStorage_GetEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetStorageMut"), STAT_EntityStorage_GetStorageMut, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::GetEntityMut"), STAT_EntityStorage_GetEntityMut, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::NewEntity"), STAT_EntityStorage_NewEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);
DECLARE_CYCLE_STAT(TEXT("EntityStorage::RemoveEntity"), STAT_EntityStorage_RemoveEntity, STATGROUP_GRAPHS_PERF_ENTITY_STORAGE);

class ES {
public:
	template <typename Entity>
	using Storage = TSparseArray<Entity>;

	ES(const ES&) = delete;
	ES(ES&&) noexcept = delete;
	ES &operator=(const ES&) = delete;
	ES &operator=(ES&&) noexcept = delete;

	template <typename Entity>
	FORCEINLINE static const Storage<Entity> &GetStorage() {
		return GetInstance().GetStorageMut<Entity>();
	}

	template <typename Entity>
	static bool IsValid(const EntityId Id) {
		SCOPE_CYCLE_COUNTER(STAT_EntityStorage_IsValidTemplate);
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
private:
	ES() = default;

	static ES &GetInstance() {
		static ES Instance;
		return Instance;
	}

	Storage<VertexEntity> Vertices;
	Storage<EdgeEntity> Edges;
	Storage<GraphEntity> Graphs;

	friend class GraphsRendererCommand;
};
