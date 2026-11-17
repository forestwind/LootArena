#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "LAGameMode.generated.h"

class ALAEnemyCharacter;
class ALAEnemySpawnPoint;
class UAbilitySystemComponent;
class ULAArenaData;
enum class ELAArenaState : uint8;
struct FLAArenaSpawnEntry;

UCLASS()
class LOOTARENA_API ALAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALAGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Arena")
	TObjectPtr<ULAArenaData> ArenaData;

private:
	// 데이터·GameState 확인 → 스폰 지점 수집 → 시작 대기
	void StartArena();

	// 이전 판 데이터 초기화 (인벤토리)
	void ResetData();

	// 레벨의 ALAEnemySpawnPoint → SpawnIdToPoint
	void CollectSpawnPoints();

	// 다음 스테이지 적 스폰
	void StartNextStage();

	// 지점 위치·플레이어 방향 스폰 → 목표 = 플레이어
	ALAEnemyCharacter* SpawnEnemy(const FLAArenaSpawnEntry& Entry, APawn* PlayerPawn);

	// 살아 있는 적 목록 추가 + 사망 태그 구독
	void TrackEnemy(ALAEnemyCharacter* Enemy);

	// 적 사망 태그 구독 해제
	void UntrackEnemy(ALAEnemyCharacter* Enemy);

	// 살아 있는 적 전체 구독 해제 + 목록 비우기
	void UntrackAllEnemies();

	// 적 Character.State.Dead 추가 시 목록에서 제거
	void OnEnemyDeadTagChanged(const FGameplayTag Tag, int32 NewCount, TWeakObjectPtr<ALAEnemyCharacter> Enemy);

	// 진행 중 + 살아 있는 적 0 → 클리어 (마지막이면 승리)
	void CheckStageCleared();

	// 플레이어 사망 태그 구독
	void TrackPlayer();

	// 플레이어 사망 태그 구독 해제
	void UntrackPlayer();

	// 플레이어 Character.State.Dead 추가 시 패배
	void OnPlayerDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 승리·패배 확정 + 대기 타이머·플레이어 구독 정리 (종료 후 무시)
	void FinishArena(ELAArenaState ResultState);

	// 대기 후 다음 스테이지 시작 (최소 0.1초)
	void ScheduleNextStage(float Delay);

	// GameState 상태·스테이지 번호 갱신
	void SetArenaState(ELAArenaState NewState);

	UPROPERTY()
	TMap<int32, TObjectPtr<ALAEnemySpawnPoint>> SpawnIdToPoint;

	// 현재 스테이지에서 살아 있는 적
	TArray<TWeakObjectPtr<ALAEnemyCharacter>> AliveEnemies;

	// 0부터, 시작 전 INDEX_NONE
	int32 CurrentStageIndex = INDEX_NONE;

	FTimerHandle StageTimerHandle;

	// 사망 태그 구독 중인 플레이어 ASC
	TWeakObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;
};
