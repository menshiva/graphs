#pragma once

enum class EntitySignature : uint8_t {
	GRAPH = 0,
	VERTEX,
	EDGE,
	SIZE
};

struct EntityId {
	static EntityId NONE() {
		static EntityId NoneId;
		return NoneId;
	}

	EntityId() : Index(-1), Signature(EntitySignature::SIZE) {}
	EntityId(const uint32_t Index, const EntitySignature Signature) : Index(Index), Signature(Signature) {}

	bool operator==(const EntityId OtherId) const {
		return Signature == OtherId.Signature && Index == OtherId.Index;
	}

	bool operator!=(const EntityId OtherId) const {
		return !(*this == OtherId);
	}
private:
	uint32_t Index;
	EntitySignature Signature;

	friend class UTool;
	friend class EntityStorage;
};
