#include "Game/LAArenaData.h"

int32 ULAArenaData::GetStageCount() const
{
	return Stages.Num();
}

bool ULAArenaData::GetStage(int32 StageIndex, FLAArenaStage& OutStage) const
{
	if (!Stages.IsValidIndex(StageIndex))
	{
		return false;
	}

	OutStage = Stages[StageIndex];
	return true;
}
