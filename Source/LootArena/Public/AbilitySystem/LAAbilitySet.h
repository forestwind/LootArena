#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "LAAbilitySet.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class ULAGameplayAbility;

// 세트로 부여한 결과 (한 번에 해제)
USTRUCT()
struct FLAAbilitySetGrantedHandles
{
	GENERATED_BODY()

	void RemoveAbilitiesAndEffects(UAbilitySystemComponent* AbilitySystemComponent);

	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;
};

UCLASS()
class LOOTARENA_API ULAAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	void GiveAbilitiesAndEffects(UAbilitySystemComponent* AbilitySystemComponent, FLAAbilitySetGrantedHandles& OutGrantedHandles) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<ULAGameplayAbility>> Abilities;

	// Has Duration·Infinite GE (해제 시 제거)
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffects;
};
