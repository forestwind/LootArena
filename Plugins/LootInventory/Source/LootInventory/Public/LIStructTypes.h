// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LIStructTypes.generated.h"

class UTexture2D;

// 공용 구조체

// 인벤토리 한 칸
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIInventoryEntry
{
	GENERATED_BODY()

public:
	// 칸 번호 (빈 번호 재사용)
	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	int32 SlotId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	int32 Count = 0;
};

// UI 표시용 아이템 정보
// UI는 ItemId만 들고 있다가 필요할 때 이 값을 받아 표시 (행 주소 대신 값 복사)
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIItemDisplayInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	int32 ItemId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FText Description;
	
	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FGameplayTag Rarity;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FGameplayTag ItemType;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	int32 MaxStack = 1;
};
