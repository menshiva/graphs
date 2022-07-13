#pragma once

#include "Components/WidgetComponent.h"
#include "MenuWidgetComponent.generated.h"

UCLASS()
class GRAPHS_API UMenuWidgetComponent final : public UWidgetComponent {
	GENERATED_BODY()
public:
	explicit UMenuWidgetComponent(const FObjectInitializer &ObjectInitializer);

	void SetVisble(bool Visible);

	void SetCursorVisibility(bool Visible) const;
	void SetCursorLocation(const FVector &Location) const;
private:
	UPROPERTY()
	UStaticMeshComponent *m_Cursor;
};
