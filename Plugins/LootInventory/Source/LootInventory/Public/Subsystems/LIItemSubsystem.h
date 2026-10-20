// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "LIStructTypes.h"
#include "LIItemSubsystem.generated.h"

class UDataTable;
class ULIItemDataAssetBase;
struct FLIItemRowBase;
struct FLIItemAssetsBase;

// 조회 맵 한 칸: 행 + 그 행이 속한 묶음
struct FLIItemEntry
{
	const FLIItemRowBase* Row = nullptr;					// 테이블 안의 행 (테이블 메모리)
	const ULIItemDataAssetBase* ItemDataAsset = nullptr;	// 리소스 조회용 (FindAssets)
};

// ItemId → 아이템 행 / 리소스 조회
UCLASS()
class LOOTINVENTORY_API ULIItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 전체 비우고 처음부터 다시 로드
	// 게임 진입 전(아이템 사용 전)에만 호출
	void ReloadItemData(const TMap<FGameplayTag, UDataTable*>& InChangeTables);

	const FLIItemRowBase* FindItemRow(int32 InItemId) const;
	const FLIItemAssetsBase* FindItemAssets(int32 InItemId) const;

	// ItemId의 UI 표시 정보
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "LootInventory")
	bool GetItemDisplayInfo(int32 InItemId, FLIItemDisplayInfo& OutInfo) const;

private:
	void AddTableRows(UDataTable* InItemTable, const ULIItemDataAssetBase* InItemDataAsset);

#if WITH_EDITOR
	// 쿡 목록에 없는 데이터 폴더 경고 (에디터 전용)
	void CheckCookDirectories() const;
#endif

	TMap<int32, FLIItemEntry> ItemIdToEntry;

	// 캐싱용 저장
	UPROPERTY()
	TArray<TObjectPtr<ULIItemDataAssetBase>> LoadedItemDataAssets;

	// 캐싱용 저장
	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedItemTables;
};