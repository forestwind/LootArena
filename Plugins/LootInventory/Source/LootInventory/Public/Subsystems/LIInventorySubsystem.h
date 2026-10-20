// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LIStructTypes.h"
#include "LIInventorySubsystem.generated.h"

// 여러 곳에서 구독 가능하고 BP에서도 바인딩되는 이벤트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLIOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLIOnItemAdded, int32, ItemId, int32, Count);

// 아이템 목록 (ItemId + Count, 행 주소는 저장하지 않음)
UCLASS()
class LOOTINVENTORY_API ULIInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// MaxStack 기준으로 기존 칸을 채우고 남으면 새 칸 추가
	UFUNCTION(BlueprintCallable, Category = "LootInventory")
	bool AddItem(int32 InItemId, int32 InCount);

	// 칸에서 1개 제거
	UFUNCTION(BlueprintCallable, Category = "LootInventory")
	bool RemoveItemFromSlot(int32 InSlotId);

	UFUNCTION(BlueprintPure, Category = "LootInventory")
	const TArray<FLIInventoryEntry>& GetEntries() const { return Entries; }

	// 목록이 바뀔 때 (UI 갱신용)
	UPROPERTY(BlueprintAssignable, Category = "LootInventory")
	FLIOnInventoryChanged OnInventoryChanged;

	// 아이템을 얻었을 때 (획득 토스트용, Count = 이번에 얻은 총 개수)
	UPROPERTY(BlueprintAssignable, Category = "LootInventory")
	FLIOnItemAdded OnItemAdded;

private:
	// 빈 칸 번호 찾기
	int32 FindFreeSlotId() const;

	UPROPERTY()
	TArray<FLIInventoryEntry> Entries;
};
