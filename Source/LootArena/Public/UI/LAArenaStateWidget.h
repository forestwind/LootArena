#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/LAGameState.h"
#include "LAArenaStateWidget.generated.h"

// 아레나 상태 표시 위젯 부모 (GameState 알림 → BP 이벤트)
UCLASS(Abstract)
class LOOTARENA_API ULAArenaStateWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 연결 시 1회 + 상태 변경마다
	UFUNCTION(BlueprintImplementableEvent, Category = "Arena")
	void OnArenaStateChanged(ELAArenaState State, int32 StageNumber, int32 StageCount);

private:
	// GameState 알림 구독 + 현재 값 반영
	void ConnectGameState();

	// 구독 해제
	void DisconnectGameState();

	void HandleArenaStateChanged();

	TWeakObjectPtr<ALAGameState> ArenaGameState;

	FDelegateHandle StateChangedHandle;
};
