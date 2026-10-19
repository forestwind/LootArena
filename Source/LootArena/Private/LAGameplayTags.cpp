#include "LAGameplayTags.h"

namespace LAGameplayTags
{
	// Input
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack, "InputTag.Attack");

	// Character State
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead, "Character.State.Dead");

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Melee_Hit, "Event.Melee.Hit");
}
