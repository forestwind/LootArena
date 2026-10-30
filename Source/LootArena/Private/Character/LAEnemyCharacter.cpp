#include "Character/LAEnemyCharacter.h"

#include "AbilitySystem/LACharacterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Character/LACharacterData.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffectTypes.h"
#include "LAGameplayTags.h"
#include "Subsystems/LILootSubsystem.h"
#include "TimerManager.h"
#include "UI/LAAttributeWidget.h"
#include "UI/LABillboardWidgetComponent.h"

ALAEnemyCharacter::ALAEnemyCharacter()
{
	// 배치·스폰 모두 AI 빙의 (웨이브 스폰 대비)
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 컨트롤러 방향(포커스)으로 회전 속도에 맞춰 회전
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

	HealthBarComponent = CreateDefaultSubobject<ULABillboardWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(GetRootComponent());
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::World);
	// 판정·시야 트레이스 제외
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALAEnemyCharacter::BeginPlay()
{
	// 위젯 컴포넌트 위젯 생성 (컴포넌트 BeginPlay)
	Super::BeginPlay();

	// 사망 태그 구독 (드롭 그룹은 드롭 시점에 확인)
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::OnDeadTagChanged);

	InitHealthBar();
}

void ALAEnemyCharacter::PossessedBy(AController* NewController)
{
	// 베이스에서 InitAbilityActorInfo 먼저
	Super::PossessedBy(NewController);

	if (CharacterData.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterData not set: %s"), *GetName());
		return;
	}

	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CharacterData.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnCharacterDataLoaded));
}

void ALAEnemyCharacter::OnCharacterDataLoaded()
{
	LoadedCharacterData = CharacterData.Get();
	if (LoadedCharacterData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterData load failed: %s"), *GetName());
		return;
	}

	ApplyCharacterData(LoadedCharacterData);
}

void ALAEnemyCharacter::HandleDeath()
{
	Super::HandleDeath();

	// BT 정지 + 컨트롤러 제거
	DetachFromControllerPendingDestroy();

	// 0 이하는 수명 해제라 최소값 보장
	SetLifeSpan(FMath::Max(DeathLifeSpan, 0.1f));
}

void ALAEnemyCharacter::OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 추가 시에만 (제거 무시)
	if (NewCount <= 0)
	{
		return;
	}

	// 사망 시 체력바 숨김
	HideHealthBar();

	if (LoadedCharacterData == nullptr || LoadedCharacterData->DropGroupId == 0)
	{
		return;
	}

	ULILootSubsystem* LootSubsystem = GetGameInstance()->GetSubsystem<ULILootSubsystem>();
	if (LootSubsystem == nullptr)
	{
		return;
	}

	// 발밑 위치
	const FVector FootLocation = GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	LootSubsystem->DropLoot(GetWorld(), LoadedCharacterData->DropGroupId, FootLocation);
}

void ALAEnemyCharacter::InitHealthBar()
{
	ULAAttributeWidget* HealthBarWidget = Cast<ULAAttributeWidget>(HealthBarComponent->GetUserWidgetObject());
	if (HealthBarWidget != nullptr)
	{
		HealthBarWidget->SetTargetActor(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthBar widget not set or not LAAttributeWidget: %s"), *GetName());
	}

	// 처음 숨김 (피격 시 표시)
	HealthBarComponent->SetVisibility(false);

	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(ULACharacterAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ThisClass::OnHealthChanged);
}

void ALAEnemyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 감소 시에만 (회복·초기 스탯 무시)
	if (Data.NewValue >= Data.OldValue)
	{
		return;
	}

	HealthBarComponent->SetVisibility(true);

	// 같은 핸들 재설정 → 타이머 재시작
	GetWorldTimerManager().SetTimer(HealthBarHideTimerHandle, this, &ThisClass::HideHealthBar, HealthBarDisplayDuration, false);
}

void ALAEnemyCharacter::HideHealthBar()
{
	GetWorldTimerManager().ClearTimer(HealthBarHideTimerHandle);

	HealthBarComponent->SetVisibility(false);
}
