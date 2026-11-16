#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "LAEnemySpawnPoint.generated.h"

// 아레나 적 스폰 위치 (스테이지 데이터 SpawnId로 매칭)
UCLASS()
class LOOTARENA_API ALAEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	int32 GetSpawnId() const { return SpawnId; }

private:
	// 레벨에서 지점마다 직접 입력 (1부터, 0 = 미입력 → 무시)
	UPROPERTY(EditInstanceOnly, Category = "Arena")
	int32 SpawnId = 0;
};
