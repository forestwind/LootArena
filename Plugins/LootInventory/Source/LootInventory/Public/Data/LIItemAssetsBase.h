// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LIItemAssetsBase.generated.h"

class UTexture2D;

// 아이템 리소스 기반
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIItemAssetsBase
{
	GENERATED_BODY()

public:
	virtual ~FLIItemAssetsBase() = default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	TSoftObjectPtr<UTexture2D> Icon;
};
