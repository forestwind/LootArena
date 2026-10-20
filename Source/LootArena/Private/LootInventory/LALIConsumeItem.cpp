#include "LootInventory/LALIConsumeItem.h"

#include "LootInventory/LALIGameplayTags.h"

FGameplayTag FLALIConsumeItemRow::GetItemType() const
{
	return LALIGameplayTags::Item_Type_Consume;
}
