#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "ListItemWidget.generated.h"

UCLASS()
// ReSharper disable once CppClassCanBeFinal
class UListItemData : public UObject {
	GENERATED_BODY()
public:
	FString DisplayText;
	FString TextData;
};

UCLASS(Abstract, meta=(DisableNativeTick))
// ReSharper disable once CppClassCanBeFinal
class GRAPHS_API UListItemWidget : public UUserWidget, public IUserObjectListEntry {
	GENERATED_BODY()
protected:
	virtual void NativeOnListItemObjectSet(UObject *ListItemObject) override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UTextBlock *DisplayText;

	UPROPERTY()
	UListItemData *Data;
};
