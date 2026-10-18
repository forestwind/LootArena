#include "AbilitySystem/LAAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LAGameplayAbility.h"
#include "GameplayEffect.h"

void FLAAbilitySetGrantedHandles::RemoveAbilitiesAndEffects(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : AppliedEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}

	GrantedAbilityHandles.Reset();
	AppliedEffectHandles.Reset();
}

void ULAAbilitySet::GiveAbilitiesAndEffects(UAbilitySystemComponent* AbilitySystemComponent, FLAAbilitySetGrantedHandles& OutGrantedHandles) const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 1. 어빌리티 등록 (발동은 입력·이벤트 시점, OnGiven만 등록 즉시 발동)
	for (const TSubclassOf<ULAGameplayAbility>& AbilityClass : Abilities)
	{
		if (AbilityClass == nullptr)
		{
			continue;
		}

		const FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		OutGrantedHandles.GrantedAbilityHandles.Add(AbilitySystemComponent->GiveAbility(AbilitySpec));
	}

	// 2. 지속형 GE 적용 (핸들 보관, 해제 시 제거)
	for (const TSubclassOf<UGameplayEffect>& EffectClass : OngoingEffects)
	{
		if (EffectClass == nullptr)
		{
			continue;
		}

		const FActiveGameplayEffectHandle EffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(
			EffectClass.GetDefaultObject(), 1.f, AbilitySystemComponent->MakeEffectContext());
		OutGrantedHandles.AppliedEffectHandles.Add(EffectHandle);
	}
}
