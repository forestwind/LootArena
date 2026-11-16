#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LAArenaData.generated.h"

class ALAEnemyCharacter;

// 적 1마리
USTRUCT(BlueprintType)
struct FLAArenaSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	TSubclassOf<ALAEnemyCharacter> EnemyClass;

	// ALAEnemySpawnPoint SpawnId와 매칭
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	int32 SpawnId = 0;
};

// 스테이지 1개
USTRUCT(BlueprintType)
struct FLAArenaStage
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	TArray<FLAArenaSpawnEntry> SpawnEntries;
};

// 아레나 구성 (스테이지 순서 = 배열 순서)
UCLASS()
class LOOTARENA_API ULAArenaData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 스테이지 조회는 이 두 함수로만 (저장 형태와 분리)
	int32 GetStageCount() const;
	bool GetStage(int32 StageIndex, FLAArenaStage& OutStage) const;

	// 시작 → 첫 스폰 대기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float StartDelay = 2.f;

	// 스테이지 클리어 → 다음 스폰 대기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	float StageClearedDelay = 3.f;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	TArray<FLAArenaStage> Stages;
};
