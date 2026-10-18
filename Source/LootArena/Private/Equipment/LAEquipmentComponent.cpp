#include "Equipment/LAEquipmentComponent.h"

#include "AbilitySystemGlobals.h"
#include "Components/StaticMeshComponent.h"
#include "Equipment/LAWeaponData.h"
#include "GameFramework/Character.h"

void ULAEquipmentComponent::EquipWeapon(const ULAWeaponData* WeaponData)
{
	if (WeaponData == nullptr)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	// 무기 하나만 유지
	UnequipWeapon();

	// 1. 메시 부착 (판정은 메시 충돌 박스 스윕이라 컴포넌트 충돌 없음)
	WeaponMeshComponent = NewObject<UStaticMeshComponent>(OwnerCharacter);
	WeaponMeshComponent->SetStaticMesh(WeaponData->Mesh);
	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComponent->SetupAttachment(OwnerCharacter->GetMesh(), WeaponData->AttachSocketName);
	WeaponMeshComponent->RegisterComponent();

	// 2. 무기 어빌리티 세트 부여
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerCharacter);
	if (AbilitySystemComponent != nullptr && WeaponData->AbilitySet != nullptr)
	{
		WeaponData->AbilitySet->GiveAbilitiesAndEffects(AbilitySystemComponent, WeaponGrantedHandles);
	}

	EquippedWeaponData = WeaponData;
}

void ULAEquipmentComponent::UnequipWeapon()
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	WeaponGrantedHandles.RemoveAbilitiesAndEffects(AbilitySystemComponent);

	if (WeaponMeshComponent != nullptr)
	{
		WeaponMeshComponent->DestroyComponent();
		WeaponMeshComponent = nullptr;
	}

	EquippedWeaponData = nullptr;
}
