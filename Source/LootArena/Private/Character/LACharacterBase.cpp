#include "Character/LACharacterBase.h"

#include "AbilitySystem/LAAbilitySystemComponent.h"
#include "AbilitySystem/LACharacterAttributeSet.h"
#include "Character/LACharacterData.h"
#include "Character/LACharacterMovementComponent.h"
#include "Combat/LAMeleeTraceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Equipment/LAEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"

ALACharacterBase::ALACharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULACharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<ULAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// 서브오브젝트 세트 → ASC InitializeComponent에서 ASC 세트 목록에 자동 등록
	AttributeSet = CreateDefaultSubobject<ULACharacterAttributeSet>(TEXT("AttributeSet"));
	EquipmentComponent = CreateDefaultSubobject<ULAEquipmentComponent>(TEXT("EquipmentComponent"));
	MeleeTraceComponent = CreateDefaultSubobject<ULAMeleeTraceComponent>(TEXT("MeleeTraceComponent"));
}

UAbilitySystemComponent* ALACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ALACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(ULACharacterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ThisClass::OnHealthChangedForFlash);
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

	// 3. 기본 무기 장착 (메시 부착 → 무기 세트 부여)
	if (CharacterData->DefaultWeapon != nullptr)
	{
		EquipmentComponent->EquipWeapon(CharacterData->DefaultWeapon);
	}
}

void ALACharacterBase::OnHealthChangedForFlash(const FOnAttributeChangeData& Data)
{
	// 감소 시에만 (회복·초기 스탯 무시)
	if (Data.NewValue >= Data.OldValue)
	{
		return;
	}

	GetMesh()->SetOverlayMaterial(HitFlashMaterial);

	// 같은 핸들 재설정 → 타이머 재시작
	GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &ThisClass::ClearHitFlash, HitFlashDuration, false);
}

void ALACharacterBase::ClearHitFlash()
{
	GetWorldTimerManager().ClearTimer(HitFlashTimerHandle);

	GetMesh()->SetOverlayMaterial(nullptr);
}

void ALACharacterBase::HandleDeath()
{
	// 시체에 오버레이 남지 않게
	ClearHitFlash();

	// 이동 먼저 정지 (충돌 끈 뒤 낙하 방지)
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
