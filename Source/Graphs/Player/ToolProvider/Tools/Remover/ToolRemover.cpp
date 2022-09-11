#include "ToolRemover.h"
#include "Graphs/GraphRenderer/Commands/EdgeCommands.h"
#include "Graphs/GraphRenderer/Commands/GraphCommands.h"
#include "Graphs/GraphRenderer/Commands/VertexCommands.h"

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
	if (IsPressed && GetHitEntityId() != EntityId::NONE()) {
		if (GetEntityStorage().IsValid<VertexEntity>(GetHitEntityId())) {
			GetGraphRenderer()->PushCommand(VertexCommands::Remove(GetHitEntityId()));
		}
		else if (GetEntityStorage().IsValid<EdgeEntity>(GetHitEntityId())) {
			GetGraphRenderer()->PushCommand(EdgeCommands::Remove(GetHitEntityId()));
		}
		else {
			check(GetEntityStorage().IsValid<GraphEntity>(GetHitEntityId()));
			GetGraphRenderer()->PushCommand(GraphCommands::Remove(GetHitEntityId()));
		}
		GetGraphRenderer()->MarkDirty();
		return true;
	}
	return Super::OnRightTriggerAction(IsPressed);
}
