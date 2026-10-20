// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LIItemViewerLibrary.generated.h"

class UDataTable;
class UTexture2D;
class ULIItemDataAssetBase;

// 필드 이름 / 값 문자열
USTRUCT(BlueprintType)
struct LOOTINVENTORY_API FLIItemViewerField
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "LootInventory")
	FString Value;
};

// 아이템 뷰어용 DataTable / DataAsset 조회 (리플렉션, 기반 → 자식 순서)
UCLASS()
class LOOTINVENTORY_API ULIItemViewerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Row Struct가 FLIItemRowBase 파생인지
	UFUNCTION(BlueprintPure, Category = "LootInventory|Viewer")
	static bool IsItemTable(const UDataTable* Table);

	// 전체 ItemId (행 순서)
	UFUNCTION(BlueprintPure, Category = "LootInventory|Viewer")
	static TArray<int32> GetTableItemIds(const UDataTable* Table);

	// Row Struct 필드 이름 (기반 → 자식)
	UFUNCTION(BlueprintPure, Category = "LootInventory|Viewer")
	static TArray<FString> GetTableFieldNames(const UDataTable* Table);

	// ItemId 행 필드 값 (GetTableFieldNames 순서)
	UFUNCTION(BlueprintCallable, Category = "LootInventory|Viewer")
	static bool GetItemRowFields(const UDataTable* Table, int32 ItemId, TArray<FLIItemViewerField>& OutFields);

	// 리소스 구조체 필드 이름 (기반 → 자식, Icon 제외)
	UFUNCTION(BlueprintPure, Category = "LootInventory|Viewer")
	static TArray<FString> GetAssetFieldNames(const ULIItemDataAssetBase* DataAsset);

	// ItemId 리소스 Icon + 필드 값 (GetAssetFieldNames 순서)
	UFUNCTION(BlueprintCallable, Category = "LootInventory|Viewer")
	static bool GetItemAssetFields(const ULIItemDataAssetBase* DataAsset, int32 ItemId,
	                               TSoftObjectPtr<UTexture2D>& OutIcon, TArray<FLIItemViewerField>& OutFields);
};
