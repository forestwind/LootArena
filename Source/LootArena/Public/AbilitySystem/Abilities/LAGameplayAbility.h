#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LAGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ELAAbilityActivationPolicy : uint8
{
	OnInputPressed,
	WhileInputHeld,
	OnGiven
};

UCLASS(Abstract)
class LOOTARENA_API ULAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULAGameplayAbility();

	const FGameplayTag& GetInputTag() const { return InputTag; }
	ELAAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, Category = "Activation")
	ELAAbilityActivationPolicy ActivationPolicy = ELAAbilityActivationPolicy::OnInputPressed;
};
