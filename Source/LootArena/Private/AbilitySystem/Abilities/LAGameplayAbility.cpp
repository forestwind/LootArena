#include "AbilitySystem/Abilities/LAGameplayAbility.h"

#include "AbilitySystemComponent.h"

ULAGameplayAbility::ULAGameplayAbility()
{
	// 캐릭터당 인스턴스 하나 (콤보 단계 등 상태 유지)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void ULAGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ActivationPolicy != ELAAbilityActivationPolicy::OnGiven)
	{
		return;
	}

	if (ActorInfo == nullptr || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
}
