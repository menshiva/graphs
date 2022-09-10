#include "ToolRemover.h"

UToolRemover::UToolRemover() : UTool(
	"Remove",
	TEXT("/Game/Graphs/UI/Icons/Remove"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolRemoverPanel")
) {
	SetSupportedEntities({EntitySignature::GRAPH, EntitySignature::EDGE, EntitySignature::VERTEX});
}

void UToolRemover::OnAttach() {
	Super::OnAttach();
	GetVrRightController()->SetLaserActive(true);
}

void UToolRemover::OnDetach() {
	Super::OnDetach();
	GetVrRightController()->SetLaserActive(false);
}

bool UToolRemover::OnRightTriggerAction(const bool IsPressed) {
	// TODO
	/*if (IsPressed && GetHitEntityId() != ENTITY_NONE) {
		const auto HitEntityType = GetGraphProvider()->GetEntityType(GetHitEntityId());
		if (HitEntityType == EntityType::VERTEX)
			GetGraphProvider()->ExecuteCommand(VertexCommands::Remove(GetHitEntityId()));
		else if (HitEntityType == EntityType::EDGE)
			GetGraphProvider()->ExecuteCommand(EdgeCommands::Remove(GetHitEntityId()));
		else {
			check(HitEntityType == EntityType::GRAPH);
			GetGraphProvider()->ExecuteCommand(GraphCommands::Remove(GetHitEntityId()));
		}
		return true;
	}*/
	return Super::OnRightTriggerAction(IsPressed);
}
