// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/LIItemRowBase.h"
#include "Data/LIItemAssetsBase.h"
#include "LIItemDataAsset.generated.h"

// 아이템 타입 묶음 (타입 태그, 아이템 테이블, ItemId → 리소스 구조체 맵)
UCLASS(Abstract, BlueprintType)
class LOOTINVENTORY_API ULIItemDataAssetBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (Categories = "LI.Item.Type"))
	FGameplayTag ItemType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UDataTable> ItemTable;

	virtual const FLIItemAssetsBase* FindAssets(int32 InItemId) const { return nullptr; }
};
