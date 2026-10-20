#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "LACharacterAttributeSet.generated.h"

// 속성 Get/Set/Init 함수 생성
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class LOOTARENA_API ULACharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, Attack)

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, Defense)

	// 받은 데미지 (Health 반영 후 0)
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, Damage)

	// 받은 회복량 (Health 반영 후 0)
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(ULACharacterAttributeSet, Healing)

protected:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

private:
	// Health 0 ~ MaxHealth
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
