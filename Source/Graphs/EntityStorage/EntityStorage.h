#pragma once

#include "Entities/GraphEntity.h"
#include "Entities/VertexEntity.h"
#include "Entities/EdgeEntity.h"

class ES {
public:
	template <typename Entity>
	using StorageImpl = TSparseArray<Entity>;

	ES() = default;
	ES(const ES&) = delete;
	ES(ES&&) noexcept = delete;
	ES &operator=(const ES&) = delete;
	ES &operator=(ES&&) noexcept = delete;

	template <typename Entity>
	static bool IsValid(const EntityId Id) {
		if (Entity::Signature != Id.GetSignature() || Id == EntityId::NONE())
			return false;
		return GetInstance().GetStorage<Entity>().IsValidIndex(Id.GetIndex());
	}

	template <typename Entity>
	FORCEINLINE static const Entity &GetEntity(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.GetSignature());
		return GetInstance().GetStorage<Entity>()[Id.GetIndex()];
	}

	template <typename Entity>
	static void Reserve(size_t NumElementsToAdd) {
		auto &Storage = GetInstance().GetStorage<Entity>();
		Storage.Reserve(Storage.Num() + NumElementsToAdd);
	}

	template <typename Entity>
	FORCEINLINE static void Clear() {
		GetInstance().GetStorage<Entity>().Empty();
	}

	template <typename Entity>
	FORCEINLINE static Entity &GetEntityMut(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.GetSignature());
		return GetInstance().GetStorage<Entity>()[Id.GetIndex()];
	}

	template <typename Entity>
	FORCEINLINE static EntityId NewEntity() {
		return {
			static_cast<uint32_t>(GetInstance().GetStorage<Entity>().Emplace()),
			Entity::Signature
		};
	}

	template <typename Entity>
	FORCEINLINE static void RemoveEntity(const EntityId Id) {
		check(Id != EntityId::NONE());
		check(Entity::Signature == Id.GetSignature());
		GetInstance().GetStorage<Entity>().RemoveAt(Id.GetIndex());
	}
private:
	static ES &GetInstance() {
		static ES Singleton;
		return Singleton;
	}

	template <typename Entity>
	FORCEINLINE StorageImpl<Entity> &GetStorage() {
		UE_LOG(LogTemp, Fatal, TEXT("Undefined type!"));
		return StorageImpl<Entity>();
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<VertexEntity> &GetStorage<VertexEntity>() {
		return Vertices;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<EdgeEntity> &GetStorage<EdgeEntity>() {
		return Edges;
	}

	template <>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	FORCEINLINE StorageImpl<GraphEntity> &GetStorage<GraphEntity>() {
		return Graphs;
	}

	StorageImpl<VertexEntity> Vertices;
	StorageImpl<EdgeEntity> Edges;
	StorageImpl<GraphEntity> Graphs;
};
