#include "ToolRemover.h"
#include "Graphs/GraphProvider/Commands/EdgeCommands.h"
#include "Graphs/GraphProvider/Commands/GraphCommands.h"
#include "Graphs/GraphProvider/Commands/VertexCommands.h"
#include "Graphs/GraphProvider/Entities/GraphEntity.h"
#include "Graphs/Utils/Consts.h"

DECLARE_CYCLE_STAT(TEXT("UToolRemover::OnRightTriggerAction"), STAT_UToolRemover_OnRightTriggerAction, STATGROUP_GRAPHS_PERF);

UToolRemover::UToolRemover() : UTool(
	"Remove",
	TEXT("/Game/Graphs/UI/Icons/Remove"),
	TEXT("/Game/Graphs/UI/Blueprints/Tools/ToolRemoverPanel")
) {
	SetSupportedEntityTypes({EntityType::GRAPH, EntityType::EDGE, EntityType::VERTEX});
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
	SCOPE_CYCLE_COUNTER(STAT_UToolRemover_OnRightTriggerAction);

	if (IsPressed && GetHitEntityId() != ENTITY_NONE) {
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
	}

	return Super::OnRightTriggerAction(IsPressed);
}
