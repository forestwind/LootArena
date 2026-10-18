#include "AbilitySystem/LAAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/LAGameplayAbility.h"

void ULAAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// 목록 순회 중 발동하지 않도록 모았다가 발동
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const ULAGameplayAbility* Ability = Cast<ULAGameplayAbility>(AbilitySpec.Ability);
		if (Ability == nullptr || !Ability->GetInputTag().MatchesTagExact(InputTag))
		{
			continue;
		}

		// 눌림 표시 (발동 직후 태스크가 확인)
		AbilitySpec.InputPressed = true;

		if (AbilitySpec.IsActive())
		{
			// 실행 중: 입력 눌림 이벤트 전달 (콤보 입력 등)
			AbilitySpecInputPressed(AbilitySpec);
			continue;
		}

		// 실행 전: 정책 확인 후 발동 대상에 추가
		const ELAAbilityActivationPolicy Policy = Ability->GetActivationPolicy();
		if (Policy == ELAAbilityActivationPolicy::OnInputPressed || Policy == ELAAbilityActivationPolicy::WhileInputHeld)
		{
			AbilitiesToActivate.Add(AbilitySpec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToActivate)
	{
		TryActivateAbility(Handle);
	}
}

void ULAAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// 목록 순회 중 취소하지 않도록 모았다가 취소
	TArray<FGameplayAbilitySpecHandle> AbilitiesToCancel;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		const ULAGameplayAbility* Ability = Cast<ULAGameplayAbility>(AbilitySpec.Ability);
		if (Ability == nullptr || !Ability->GetInputTag().MatchesTagExact(InputTag))
		{
			continue;
		}

		// 뗌 표시
		AbilitySpec.InputPressed = false;

		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		// 실행 중: 입력 뗌 이벤트 전달
		AbilitySpecInputReleased(AbilitySpec);

		// WhileInputHeld만 취소 (OnInputPressed는 어빌리티가 스스로 종료하기때문에 release시 취소하지 않음)
		if (Ability->GetActivationPolicy() == ELAAbilityActivationPolicy::WhileInputHeld)
		{
			AbilitiesToCancel.Add(AbilitySpec.Handle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToCancel)
	{
		CancelAbilityHandle(Handle);
	}
}
