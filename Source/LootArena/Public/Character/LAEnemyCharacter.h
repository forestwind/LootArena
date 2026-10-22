#pragma once

#include "CoreMinimal.h"
#include "Character/LACharacterBase.h"
#include "GameplayTagContainer.h"
#include "LAEnemyCharacter.generated.h"

class ULACharacterData;
class ULABillboardWidgetComponent;

UCLASS()
class LOOTARENA_API ALAEnemyCharacter : public ALACharacterBase
{
	GENERATED_BODY()

public:
	ALAEnemyCharacter();

	virtual void HandleDeath() override;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

private:
	void OnCharacterDataLoaded();

	// Character.State.Dead 추가 시 드롭
	void OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	// 체력바 위젯 → 자신 ASC 연결
	void InitHealthBar();

	// 비동기 로드 (여러 마리 스폰 시 끊김 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSoftObjectPtr<ULACharacterData> CharacterData;

	// 로드 후 유지 (GC 방지)
	UPROPERTY(Transient)
	TObjectPtr<ULACharacterData> LoadedCharacterData;

	// 사망 후 제거까지 시간 (사망 몽타주보다 길게)
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathLifeSpan = 5.f;

	// 머리 위 체력바 (위젯 클래스·크기·높이는 BP)
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<ULABillboardWidgetComponent> HealthBarComponent;
};
