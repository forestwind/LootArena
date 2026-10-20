#include "AbilitySystem/Abilities/LAUseItemAbility.h"

#include "Data/LIItemRowBase.h"
#include "Engine/GameInstance.h"
#include "GameplayEffect.h"
#include "LAGameplayTags.h"
#include "LootInventory/LALIConsumeItem.h"
#include "LootInventory/LALIGameplayTags.h"
#include "Subsystems/LIInventorySubsystem.h"
#include "Subsystems/LIItemSubsystem.h"

ULAUseItemAbility::ULAUseItemAbility()
{
	// Event.Item.Use 수신 시 발동
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = LAGameplayTags::Event_Item_Use;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void ULAUseItemAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UGameInstance* GameInstance = GetAvatarActorFromActorInfo()->GetGameInstance();
	ULIInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<ULIInventorySubsystem>();
	const ULIItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<ULIItemSubsystem>();
	if (TriggerEventData == nullptr || InventorySubsystem == nullptr || ItemSubsystem == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// SlotId는 정수로만 전송 (소수면 보낸 쪽 오류)
	const float SlotMagnitude = TriggerEventData->EventMagnitude;
	if (SlotMagnitude != FMath::FloorToFloat(SlotMagnitude))
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem: SlotId is not an integer (%f)"), SlotMagnitude);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const int32 SlotId = static_cast<int32>(SlotMagnitude);

	// 칸 → ItemId
	FLIInventoryEntry Entry;
	if (!InventorySubsystem->FindEntryBySlotId(SlotId, Entry))
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem: empty SlotId %d"), SlotId);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const int32 ItemId = Entry.ItemId;

	// 소비 타입만 사용 가능
	const FLIItemRowBase* ItemRow = ItemSubsystem->FindItemRow(ItemId);
	if (ItemRow == nullptr || ItemRow->GetItemType() != LALIGameplayTags::Item_Type_Consume)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem: ItemId %d is not a consume item"), ItemId);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 소비 타입 확인 후 리소스 변환
	const FLALIConsumeItemAssets* ConsumeAssets = static_cast<const FLALIConsumeItemAssets*>(ItemSubsystem->FindItemAssets(ItemId));
	const TSubclassOf<UGameplayEffect> UseEffectClass = ConsumeAssets != nullptr ? ConsumeAssets->UseEffect.LoadSynchronous() : nullptr;
	if (UseEffectClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem: no UseEffect for ItemId %d"), ItemId);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1개 제거 후 효과 적용
	if (InventorySubsystem->RemoveItemFromSlot(SlotId))
	{
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, MakeOutgoingGameplayEffectSpec(UseEffectClass, GetAbilityLevel()));
		UE_LOG(LogTemp, Log, TEXT("Used ItemId %d (SlotId %d)"), ItemId, SlotId);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
