// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LILootSettings.generated.h"

class ULIItemDataAssetBase;
class UDataTable;
class ALIItemPickup;

// Config/DefaultGame.ini에 저장
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Loot Inventory"))
class LOOTINVENTORY_API ULILootSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Project Settings 안의 분류 → "Plugins" 아래에 표시
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	// 타입별 묶음 DataAsset 목록 (DA_ItemConsume 등)
	UPROPERTY(Config, EditAnywhere, Category = "Item")
	TArray<TSoftObjectPtr<ULIItemDataAssetBase>> ItemDataAssets;

	// 드롭 후보 테이블 기본값
	// RequiredAssetDataTags → 행 구조체가 FLIDropRow인 테이블만 선택 창에 표시
	UPROPERTY(Config, EditAnywhere, Category = "Drop", meta = (RequiredAssetDataTags = "RowStructure=/Script/LootInventory.LIDropRow"))
	TSoftObjectPtr<UDataTable> DropTable;

	// 드롭 시 생성할 픽업 (기본값: 플러그인의 BP_ItemPickup_Base, 끝의 _C = BP가 만든 클래스)
	UPROPERTY(Config, EditAnywhere, Category = "Drop")
	TSoftClassPtr<ALIItemPickup> PickupClass = TSoftClassPtr<ALIItemPickup>(FSoftObjectPath(TEXT("/LootInventory/Pickup/BP_ItemPickup_Base.BP_ItemPickup_Base_C")));

	// 픽업을 흩뿌리는 반경 (아이템 여러 개가 한 점에 겹치지 않게)
	UPROPERTY(Config, EditAnywhere, Category = "Drop")
	float DropScatterRadius = 100.f;
};