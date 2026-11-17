#include "UI/LAArenaStateWidget.h"

#include "Engine/World.h"

void ULAArenaStateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ConnectGameState();
}

void ULAArenaStateWidget::NativeDestruct()
{
	DisconnectGameState();

	Super::NativeDestruct();
}

void ULAArenaStateWidget::ConnectGameState()
{
	// 재구성 시 중복 구독 방지
	DisconnectGameState();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	ALAGameState* FoundGameState = World->GetGameState<ALAGameState>();
	if (FoundGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState is not LAGameState: %s"), *GetName());
		return;
	}

	ArenaGameState = FoundGameState;
	StateChangedHandle = FoundGameState->OnArenaStateChanged.AddUObject(this, &ThisClass::HandleArenaStateChanged);

	// 위젯 생성 전 상태 반영
	HandleArenaStateChanged();
}

void ULAArenaStateWidget::DisconnectGameState()
{
	ALAGameState* ConnectedGameState = ArenaGameState.Get();
	if (ConnectedGameState != nullptr)
	{
		ConnectedGameState->OnArenaStateChanged.Remove(StateChangedHandle);
	}

	ArenaGameState.Reset();
	StateChangedHandle.Reset();
}

void ULAArenaStateWidget::HandleArenaStateChanged()
{
	const ALAGameState* ConnectedGameState = ArenaGameState.Get();
	if (ConnectedGameState == nullptr)
	{
		return;
	}

	OnArenaStateChanged(ConnectedGameState->GetArenaState(), ConnectedGameState->GetStageNumber(), ConnectedGameState->GetStageCount());
}
