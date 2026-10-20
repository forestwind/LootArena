#include "Character/LAEnemyCharacter.h"

#include "Character/LACharacterData.h"
#include "Engine/AssetManager.h"

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
