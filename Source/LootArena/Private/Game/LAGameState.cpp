#include "Game/LAGameState.h"

void ALAGameState::SetStageCount(int32 NewStageCount)
{
	StageCount = NewStageCount;
}

void ALAGameState::SetArenaState(ELAArenaState NewState, int32 NewStageNumber)
{
	ArenaState = NewState;
	StageNumber = NewStageNumber;

	OnArenaStateChanged.Broadcast();
}

bool ALAGameState::IsArenaFinished() const
{
	return ArenaState == ELAArenaState::Victory || ArenaState == ELAArenaState::Defeat;
}
