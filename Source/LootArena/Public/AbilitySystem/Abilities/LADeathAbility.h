#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LAGameplayAbility.h"
#include "LADeathAbility.generated.h"

class UAnimMontage;
class UGameplayEffect;

// Event.Death로 발동, 사망 상태 적용·정리·사망 몽타주 (종료 없음)
UCLASS(Abstract)
class LOOTARENA_API ULADeathAbility : public ULAGameplayAbility
{
	GENERATED_BODY()

public:
	ULADeathAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 사망 상태 GE (Infinite, Character.State.Dead 부여)
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TSubclassOf<UGameplayEffect> DeadEffect;

	// 사망 몽타주 (Enable Auto Blend Out 끔 → 마지막 자세 유지)
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;
};
