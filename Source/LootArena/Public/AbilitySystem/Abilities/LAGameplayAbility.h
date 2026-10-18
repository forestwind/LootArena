#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LAGameplayAbility.generated.h"

UENUM(BlueprintType)
enum class ELAAbilityActivationPolicy : uint8
{
	// 누를 때 발동, 어빌리티가 스스로 종료
	OnInputPressed,
	// 누를 때 발동, 뗄 때 취소
	WhileInputHeld,
	// 부여 즉시 발동, 입력 무관
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
