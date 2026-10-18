#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LAAbilitySystemComponent.generated.h"

UCLASS()
class LOOTARENA_API ULAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
};
