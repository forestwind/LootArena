#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/LAAbilitySet.h"
#include "LAEquipmentComponent.generated.h"

class UStaticMeshComponent;
class ULAWeaponData;

UCLASS()
class LOOTARENA_API ULAEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void EquipWeapon(const ULAWeaponData* WeaponData);
	void UnequipWeapon();

	UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<const ULAWeaponData> EquippedWeaponData;

	FLAAbilitySetGrantedHandles WeaponGrantedHandles;
};
