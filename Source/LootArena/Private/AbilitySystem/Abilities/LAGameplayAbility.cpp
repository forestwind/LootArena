#include "AbilitySystem/Abilities/LAGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "LAGameplayTags.h"

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

bool ULAGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 사망 상태 발동 차단 (BP 설정과 무관)
	if (!bActivatableWhenDead && AbilitySystemComponent.HasMatchingGameplayTag(LAGameplayTags::Character_State_Dead))
	{
		if (OptionalRelevantTags != nullptr)
		{
			OptionalRelevantTags->AddTag(LAGameplayTags::Character_State_Dead);
		}
		return false;
	}

	return Super::DoesAbilitySatisfyTagRequirements(AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags);
}
