#include "AbilitySystem/Abilities/LAHitReactAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "LAGameplayTags.h"

ULAHitReactAbility::ULAHitReactAbility()
{
	// Event.HitReact 수신 시 발동
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = LAGameplayTags::Event_HitReact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// 실행 중 경직 상태 (종료 시 자동 제거)
	ActivationOwnedTags.AddTag(LAGameplayTags::Character_State_HitReact);
}

void ULAHitReactAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (HitReactMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("HitReactMontage not set: %s"), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, HitReactMontage);
	// 블렌드 아웃에서 종료 (OnCompleted 미사용, 몽타주 Enable Auto Blend Out 필수)
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void ULAHitReactAbility::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void ULAHitReactAbility::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
