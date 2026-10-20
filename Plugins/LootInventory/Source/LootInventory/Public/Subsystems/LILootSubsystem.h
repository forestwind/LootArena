// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LIStructTypes.h"
#include "LILootSubsystem.generated.h"

class UDataTable;
struct FLIDropRow;

// 드롭 그룹 굴림
UCLASS()
class LOOTINVENTORY_API ULILootSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void SetDropTable(UDataTable* InDropTable);

	// 드롭 계산
	TArray<FLIInventoryEntry> RollDrops(int32 InDropGroupId) const;

	// 그룹을 굴려 결과마다 픽업 생성 (InWorld: 픽업을 놓을 맵)
	void DropLoot(UWorld* InWorld, int32 InDropGroupId, const FVector& InLocation);

private:
	// key : DropGroupId
	TMap<int32, TArray<const FLIDropRow*>> DropGroupIdToRows;
	
	UPROPERTY()
	TObjectPtr<UDataTable> LoadedDropTable;
};
