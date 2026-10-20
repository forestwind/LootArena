#pragma once

#include "CoreMinimal.h"
#include "Character/LACharacterBase.h"
#include "LAEnemyCharacter.generated.h"

class ULACharacterData;

UCLASS()
class LOOTARENA_API ALAEnemyCharacter : public ALACharacterBase
{
	GENERATED_BODY()

public:
	virtual void HandleDeath() override;

protected:
	virtual void PossessedBy(AController* NewController) override;

private:
	void OnCharacterDataLoaded();

	// 비동기 로드 (여러 마리 스폰 시 끊김 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSoftObjectPtr<ULACharacterData> CharacterData;

	// 로드 후 유지 (GC 방지)
	UPROPERTY(Transient)
	TObjectPtr<ULACharacterData> LoadedCharacterData;

	// 사망 후 제거까지 시간 (사망 몽타주보다 길게)
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathLifeSpan = 5.f;
};
