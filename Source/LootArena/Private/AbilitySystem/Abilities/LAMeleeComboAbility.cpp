#include "AbilitySystem/Abilities/LAMeleeComboAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"
#include "LAGameplayTags.h"

ULAMeleeComboAbility::ULAMeleeComboAbility()
{
	InputTag = LAGameplayTags::InputTag_Attack;
}

void ULAMeleeComboAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ComboIndex = 0;
	PlayComboMontage();
}

void ULAMeleeComboAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 유효한 종료일 때만 초기화 (중복 호출 무시), 다음 발동은 1단계부터
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		ComboIndex = 0;
		bNextComboInputPressed = false;
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void ULAMeleeComboAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	bNextComboInputPressed = true;
}

void ULAMeleeComboAbility::PlayComboMontage()
{
	if (!ComboMontages.IsValidIndex(ComboIndex) || ComboMontages[ComboIndex] == nullptr)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 이전 단계 태스크 종료 (이전 몽타주 콜백 차단)
	if (MontageTask != nullptr)
	{
		MontageTask->EndTask();
	}

	bNextComboInputPressed = false;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontages[ComboIndex]);
	// 블렌드 아웃에서 진행·종료 (OnCompleted 미사용, 몽타주 Enable Auto Blend Out 필수)
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	MontageTask->ReadyForActivation();
}

void ULAMeleeComboAbility::OnMontageBlendOut()
{
	// 버퍼 입력 있음 + 다음 단계 있음 → 다음 단계
	if (bNextComboInputPressed && ComboMontages.IsValidIndex(ComboIndex + 1))
	{
		ComboIndex++;
		PlayComboMontage();
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void ULAMeleeComboAbility::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
