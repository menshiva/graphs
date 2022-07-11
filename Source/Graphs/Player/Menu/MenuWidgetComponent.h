#pragma once

#include "Components/WidgetComponent.h"
#include "MenuWidgetComponent.generated.h"

UCLASS()
class GRAPHS_API UMenuWidgetComponent final : public UWidgetComponent {
	GENERATED_BODY()
public:
	explicit UMenuWidgetComponent(const FObjectInitializer &ObjectInitializer);

	void SetVisble(bool Visible);

	FORCEINLINE void SetCursorVisibility(bool Visible) const;
	FORCEINLINE void SetCursorLocation(const FVector &Location) const;
private:
	UPROPERTY()
	UStaticMeshComponent *m_Cursor;
};
