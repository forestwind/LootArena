#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LACharacterData.generated.h"

class UGameplayEffect;
class ULAAbilitySet;
class ULAWeaponData;

UCLASS()
class LOOTARENA_API ULACharacterData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 2순위: 어빌리티 등록 + 지속형 GE (InitialEffects 다음)
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TObjectPtr<ULAAbilitySet>> AbilitySets;

	// 1순위: 초기 스탯 등 Instant GE (AbilitySets보다 먼저)
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	// 3순위: 기본 무기 (AbilitySets 다음 장착)
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TObjectPtr<ULAWeaponData> DefaultWeapon;

	// 사망 시 드롭 그룹 (LootInventory 드롭 테이블, 0이면 드롭 없음)
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	int32 DropGroupId = 0;
};
