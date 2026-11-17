#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LAGameState.generated.h"

UENUM(BlueprintType)
enum class ELAArenaState : uint8
{
	WaitingToStart,
	InProgress,
	StageCleared,
	Victory,
	Defeat
};

DECLARE_MULTICAST_DELEGATE(FLAOnArenaStateChanged);

// 아레나 진행 상태 (값 변경은 GameMode만)
UCLASS()
class LOOTARENA_API ALAGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	// 아레나 시작 시 1회
	void SetStageCount(int32 NewStageCount);

	// 상태·스테이지 변경 → 알림
	void SetArenaState(ELAArenaState NewState, int32 NewStageNumber);

	UFUNCTION(BlueprintPure, Category = "Arena")
	ELAArenaState GetArenaState() const { return ArenaState; }

	// 1부터, 시작 전 0
	UFUNCTION(BlueprintPure, Category = "Arena")
	int32 GetStageNumber() const { return StageNumber; }

	UFUNCTION(BlueprintPure, Category = "Arena")
	int32 GetStageCount() const { return StageCount; }

	// 승리·패배 확정 여부
	UFUNCTION(BlueprintPure, Category = "Arena")
	bool IsArenaFinished() const;

	FLAOnArenaStateChanged OnArenaStateChanged;

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Arena")
	ELAArenaState ArenaState = ELAArenaState::WaitingToStart;

	UPROPERTY(VisibleInstanceOnly, Category = "Arena")
	int32 StageNumber = 0;

	UPROPERTY(VisibleInstanceOnly, Category = "Arena")
	int32 StageCount = 0;
};
