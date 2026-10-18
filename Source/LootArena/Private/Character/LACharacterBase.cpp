#include "Character/LACharacterBase.h"

#include "AbilitySystem/LAAbilitySystemComponent.h"
#include "AbilitySystem/LACharacterAttributeSet.h"
#include "Character/LACharacterData.h"
#include "GameplayEffect.h"

ALACharacterBase::ALACharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<ULAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<ULACharacterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ALACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ALACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 소유자·아바타 모두 이 캐릭터
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ALACharacterBase::ApplyCharacterData(const ULACharacterData* CharacterData)
{
	if (CharacterData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterData not set: %s"), *GetName());
		return;
	}

	// 재빙의 시 중복 부여 방지
	if (bCharacterDataApplied)
	{
		return;
	}
	bCharacterDataApplied = true;

	// 1. 초기 스탯 적용 (Instant GE, 되돌림 없음)
	// OnGiven 어빌리티가 스탯을 읽을 수 있도록 세트보다 먼저
	for (const TSubclassOf<UGameplayEffect>& EffectClass : CharacterData->InitialEffects)
	{
		if (EffectClass == nullptr)
		{
			continue;
		}

		AbilitySystemComponent->ApplyGameplayEffectToSelf(
			EffectClass.GetDefaultObject(), 1.f, AbilitySystemComponent->MakeEffectContext());
	}

	// 2. 어빌리티 세트 부여 (세트별: 어빌리티 등록 → 지속형 GE 적용)
	for (const ULAAbilitySet* AbilitySet : CharacterData->AbilitySets)
	{
		if (AbilitySet == nullptr)
		{
			continue;
		}

		AbilitySet->GiveAbilitiesAndEffects(AbilitySystemComponent, CharacterDataGrantedHandles);
	}
}
