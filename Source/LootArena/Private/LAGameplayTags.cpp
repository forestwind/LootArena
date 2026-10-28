#include "LAGameplayTags.h"

namespace LAGameplayTags
{
	// Input
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Attack, "InputTag.Attack");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Inventory, "InputTag.Inventory");

	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Melee, "Ability.Attack.Melee");

	// Character State
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Dead, "Character.State.Dead");

	// Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Melee_Hit, "Event.Melee.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Item_Use, "Event.Item.Use");

	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage_Base, "Data.Damage.Base");
}
