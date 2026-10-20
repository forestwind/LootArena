// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "LIItemRowBase.generated.h"

// 아이템 행 기반
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIItemRowBase : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory", meta = (Categories = "LI.Item.Rarity"))
	FGameplayTag Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 MaxStack = 1;

	// 행 구조체별 아이템 타입 태그
	virtual FGameplayTag GetItemType() const { return FGameplayTag(); }
};
