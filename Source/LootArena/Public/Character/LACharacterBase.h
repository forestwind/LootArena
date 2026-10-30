#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystem/LAAbilitySet.h"
#include "LACharacterBase.generated.h"

class ULAAbilitySystemComponent;
class ULACharacterAttributeSet;
class ULACharacterData;
class ULAEquipmentComponent;
class ULAMeleeTraceComponent;

UCLASS(Abstract)
class LOOTARENA_API ALACharacterBase : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ALACharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 팀 번호
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	// 캐릭터 데이터 적용 완료 여부 (판정 대상 확인용, 로드 확인용)
	bool IsCharacterDataApplied() const { return bCharacterDataApplied; }

	// 사망 정리 (이동 정지, 충돌 끄기)
	virtual void HandleDeath();

protected:
	virtual void PossessedBy(AController* NewController) override;

	// InitAbilityActorInfo 이후 호출
	void ApplyCharacterData(const ULACharacterData* CharacterData);

	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<ULAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<ULACharacterAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, Category = "Equipment")
	TObjectPtr<ULAEquipmentComponent> EquipmentComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<ULAMeleeTraceComponent> MeleeTraceComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	FGenericTeamId TeamId;

private:
	FLAAbilitySetGrantedHandles CharacterDataGrantedHandles;
	bool bCharacterDataApplied = false;
};
