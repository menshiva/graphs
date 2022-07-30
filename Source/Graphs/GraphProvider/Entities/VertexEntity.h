#pragma once

#include "Entity.h"
#include "Engine/StaticMeshActor.h"

class VertexEntity final : public Entity {
public:
	explicit VertexEntity(const class AGraphProvider &Provider);

	virtual AStaticMeshActor *GetActor() override;
	void SetActorColor(const FLinearColor &NewColor);

	EntityId GraphId = ENTITY_NONE;
};
