#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/LAAbilitySet.h"
#include "LACharacterBase.generated.h"

class ULAAbilitySystemComponent;
class ULACharacterAttributeSet;
class ULACharacterData;
class ULAEquipmentComponent;

UCLASS(Abstract)
class LOOTARENA_API ALACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ALACharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void PossessedBy(AController* NewController) override;

	// InitAbilityActorInfo 이후 호출
	void ApplyCharacterData(const ULACharacterData* CharacterData);

	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<ULAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<ULACharacterAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<ULAEquipmentComponent> EquipmentComponent;

private:
	FLAAbilitySetGrantedHandles CharacterDataGrantedHandles;
	bool bCharacterDataApplied = false;
};
