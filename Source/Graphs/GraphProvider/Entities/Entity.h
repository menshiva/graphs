#pragma once

enum class EntityType : uint8_t {
	GRAPH = 0,
	VERTEX,
	EDGE
};

enum class SelectionType : uint8_t {
	NONE = 0,
	HIT,
	SELECTED
};

using EntityId = uint32_t;
constexpr static EntityId ENTITY_NONE = static_cast<EntityId>(-1);

class Entity {
public:
	explicit Entity(AActor *Actor, const EntityType Type) : Actor(Actor), Type(Type) {}
	virtual ~Entity() = default;

	virtual AActor *GetActor() { return Actor.Get(); }
	FORCEINLINE EntityId GetId() const { return Actor->GetUniqueID(); }
	FORCEINLINE EntityType GetType() const { return Type; }

	SelectionType Selection = SelectionType::NONE;
private:
	TWeakObjectPtr<AActor> Actor;
	EntityType Type;
};

struct EntityKeyFuncs : BaseKeyFuncs<TUniquePtr<Entity>, EntityId, false> {
	using KeyInitType = TTypeTraits<EntityId>::ConstPointerType;
	using ElementInitType = TCallTraits<TUniquePtr<Entity>>::ParamType;

	static FORCEINLINE KeyInitType GetSetKey(const ElementInitType Element) {
		return Element->GetId();
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
