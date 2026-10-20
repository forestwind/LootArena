// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/LIItemDataAsset.h"
#include "LISampleItem.generated.h"

// 샘플 타입 행
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLISampleItemRow : public FLIItemRowBase
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetItemType() const override;

	// 타입에 필요한 기획 데이터 필드를 여기에 추가 (CSV 컬럼이 됨)
};

// 샘플 타입 리소스
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLISampleItemAssets : public FLIItemAssetsBase
{
	GENERATED_BODY()

	// 타입에 필요한 리소스 필드를 여기에 추가 (예: Gameplay Effect, 메시)
};

// 샘플 타입 묶음
UCLASS()
class LOOTINVENTORY_API ULISampleItemDataAsset : public ULIItemDataAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TMap<int32, FLISampleItemAssets> Assets;

	virtual const FLIItemAssetsBase* FindAssets(int32 InItemId) const override { return Assets.Find(InItemId); }
};
