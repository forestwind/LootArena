#pragma once

#include "CoreMinimal.h"
#include "Character/LACharacterBase.h"
#include "GameplayTagContainer.h"
#include "LAEnemyCharacter.generated.h"

class ULACharacterData;
class ULABillboardWidgetComponent;
struct FOnAttributeChangeData;

UCLASS()
class LOOTARENA_API ALAEnemyCharacter : public ALACharacterBase
{
	GENERATED_BODY()

public:
	ALAEnemyCharacter(const FObjectInitializer& ObjectInitializer);

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

	// Health 감소 시 체력바 표시 + 숨김 타이머 재시작
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// 체력바 숨김 + 타이머 정리
	void HideHealthBar();

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

	// 마지막 피격 후 체력바 표시 시간
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HealthBarDisplayDuration = 3.f;

	FTimerHandle HealthBarHideTimerHandle;
};
