#pragma once

enum class EntityType : uint8_t {
	GRAPH = 1,
	VERTEX = 2,
	EDGE = 4
};

enum class SelectionType : uint8_t {
	NONE = 0,
	HIT,
	SELECTED
};

using EntityId = uint32_t;
constexpr static EntityId ENTITY_NONE = static_cast<EntityId>(-1);

struct Entity {
	Entity(AActor *Actor, const EntityType Type) : Actor(Actor), Type(Type) {}
	virtual ~Entity() = default;

	FORCEINLINE EntityId GetEntityId() const { return Actor->GetUniqueID(); }

	const TWeakObjectPtr<AActor> Actor;
	const EntityType Type;

	SelectionType Selection = SelectionType::NONE;
};

struct EntityKeyFuncs : BaseKeyFuncs<TUniquePtr<Entity>, EntityId, false> {
	using KeyInitType = TTypeTraits<EntityId>::ConstPointerType;
	using ElementInitType = TCallTraits<TUniquePtr<Entity>>::ParamType;

	static FORCEINLINE KeyInitType GetSetKey(const ElementInitType Element) {
		return Element->GetEntityId();
	}

	static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B) {
		return A == B;
	}

	template <typename ComparableKey>
	static FORCEINLINE bool Matches(KeyInitType A, ComparableKey B) {
		return A == B;
	}

	static FORCEINLINE uint32 GetKeyHash(KeyInitType Key) {
		return GetTypeHash(Key);
	}

	template <typename ComparableKey>
	static FORCEINLINE uint32 GetKeyHash(ComparableKey Key) {
		return GetTypeHash(Key);
	}
};
