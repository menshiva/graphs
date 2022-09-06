#include "ListItemWidget.h"
#include "Components/TextBlock.h"

void UListItemWidget::NativeOnListItemObjectSet(UObject *ListItemObject) {
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	const auto Item = Cast<UListItemData>(ListItemObject);
	DisplayText->SetText(FText::FromString(Item->DisplayText));
	Data = Item;
}
