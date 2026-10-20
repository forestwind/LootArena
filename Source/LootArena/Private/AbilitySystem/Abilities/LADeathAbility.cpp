#include "AbilitySystem/Abilities/LADeathAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "Character/LACharacterBase.h"
#include "LAGameplayTags.h"

ULADeathAbility::ULADeathAbility()
{
	// Dead 태그 유무와 무관하게 발동
	bActivatableWhenDead = true;

	// Event.Death 수신 시 발동
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = LAGameplayTags::Event_Death;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void ULADeathAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 사망 상태 (Character.State.Dead)
	if (DeadEffect != nullptr)
	{
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, MakeOutgoingGameplayEffectSpec(DeadEffect, GetAbilityLevel()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DeadEffect not set: %s"), *GetName());
	}

	// 2. 실행 중인 다른 어빌리티 취소 (공격 중 사망)
	GetAbilitySystemComponentFromActorInfo()->CancelAllAbilities(this);

	// 3. 캐릭터 정리 (이동·충돌·제거/입력)
	ALACharacterBase* Character = Cast<ALACharacterBase>(GetAvatarActorFromActorInfo());
	if (Character != nullptr)
	{
		Character->HandleDeath();
	}

	// 4. 사망 몽타주 (어빌리티 종료 없음 → 마지막 자세 유지)
	if (DeathMontage != nullptr)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DeathMontage);
		MontageTask->ReadyForActivation();
	}
}
