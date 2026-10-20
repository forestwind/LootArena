#include "Character/LAEnemyCharacter.h"

#include "AbilitySystemComponent.h"
#include "Character/LACharacterData.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "LAGameplayTags.h"
#include "Subsystems/LILootSubsystem.h"

void ALAEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 사망 태그 구독 (드롭 그룹은 드롭 시점에 확인)
	GetAbilitySystemComponent()->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::OnDeadTagChanged);
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

	SetLifeSpan(DeathLifeSpan);
}

void ALAEnemyCharacter::OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 추가 시에만 (제거 무시)
	if (NewCount <= 0)
	{
		return;
	}

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
