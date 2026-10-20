#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LAGameplayAbility.h"
#include "LAUseItemAbility.generated.h"

// Event.Item.Use로 발동 (EventMagnitude = SlotId), 1개 제거 후 UseEffect 적용
UCLASS()
class LOOTARENA_API ULAUseItemAbility : public ULAGameplayAbility
{
	GENERATED_BODY()

public:
	ULAUseItemAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
