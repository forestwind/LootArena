#include "AbilitySystem/LACharacterAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "LAGameplayTags.h"

void ULACharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void ULACharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttribute(Attribute, NewValue);
}

void ULACharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 사망 대상은 데미지·회복 무시
	const bool bIsDead = Data.Target.HasMatchingGameplayTag(LAGameplayTags::Character_State_Dead);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage < 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Negative Damage ignored: %f"), LocalDamage);
		}
		else if (!bIsDead && LocalDamage > 0.f)
		{
			SetHealth(GetHealth() - LocalDamage);
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		const float LocalHealing = GetHealing();
		SetHealing(0.f);

		if (LocalHealing < 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Negative Healing ignored: %f"), LocalHealing);
		}
		else if (!bIsDead && LocalHealing > 0.f)
		{
			SetHealth(GetHealth() + LocalHealing);
		}
	}
}

void ULACharacterAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}
