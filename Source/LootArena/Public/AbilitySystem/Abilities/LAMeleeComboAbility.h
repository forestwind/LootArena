#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LAGameplayAbility.h"
#include "LAMeleeComboAbility.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

UCLASS(Abstract)
class LOOTARENA_API ULAMeleeComboAbility : public ULAGameplayAbility
{
	GENERATED_BODY()

public:
	ULAMeleeComboAbility();

	int32 GetComboIndex() const { return ComboIndex; }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	// 콤보 단계 순서
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontages;

private:
	void PlayComboMontage();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageInterrupted();

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	// 현재 단계 (0부터)
	int32 ComboIndex = 0;

	// 다음 단계 입력 버퍼
	bool bNextComboInputPressed = false;
};
