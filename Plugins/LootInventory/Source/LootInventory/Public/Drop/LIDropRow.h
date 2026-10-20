// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LIDropRow.generated.h"

// 드롭 후보 한 줄
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIDropRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 DropGroupId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 ItemId = 0;

	// 만분률 (10000 = 100%)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 ChancePermyriad = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 MaxCount = 1;
};