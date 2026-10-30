#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LAGameplayAbility.h"
#include "LAHitReactAbility.generated.h"

class UAnimMontage;

// Event.HitReact로 발동, 피격 몽타주 재생 (경직 상태 태그 부여)
UCLASS(Abstract)
class LOOTARENA_API ULAHitReactAbility : public ULAGameplayAbility
{
	GENERATED_BODY()

public:
	ULAHitReactAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 피격 몽타주 (Enable Auto Blend Out 필수)
	UPROPERTY(EditDefaultsOnly, Category = "HitReact")
	TObjectPtr<UAnimMontage> HitReactMontage;

private:
	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageInterrupted();
};
