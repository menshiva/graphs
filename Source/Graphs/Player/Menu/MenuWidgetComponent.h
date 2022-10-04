#pragma once

#include "Components/WidgetComponent.h"
#include "MenuWidgetComponent.generated.h"

UCLASS()
class GRAPHS_API UMenuWidgetComponent final : public UWidgetComponent {
	GENERATED_BODY()
public:
	explicit UMenuWidgetComponent(const FObjectInitializer &ObjectInitializer);

	FORCEINLINE void SetCursorVisibility(const bool Visible) const { Cursor->SetVisibility(Visible); }
	FORCEINLINE void SetCursorLocation(const FVector &Location) const { Cursor->SetWorldLocation(Location); }
private:
	UPROPERTY()
	UStaticMeshComponent *Cursor;
};
