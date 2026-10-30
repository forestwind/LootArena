#include "Character/LACharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "LAGameplayTags.h"

float ULACharacterMovementComponent::GetMaxSpeed() const
{
	if (IsHitReacting())
	{
		return 0.f;
	}

	return Super::GetMaxSpeed();
}

FRotator ULACharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	if (IsHitReacting())
	{
		return FRotator::ZeroRotator;
	}

	return Super::GetDeltaRotation(DeltaTime);
}

bool ULACharacterMovementComponent::IsHitReacting() const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (ASC == nullptr)
	{
		return false;
	}

	return ASC->HasMatchingGameplayTag(LAGameplayTags::Character_State_HitReact);
}
