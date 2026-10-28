#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "LAAIController.generated.h"

class UAbilitySystemComponent;
class UBehaviorTree;

UCLASS()
class LOOTARENA_API ALAAIController : public AAIController
{
	GENERATED_BODY()

public:
	ALAAIController();

	// 팀 = 조종 중인 폰의 TeamId (폰 없으면 NoTeam)
	virtual FGenericTeamId GetGenericTeamId() const override;

	// 팀 원본은 캐릭터 → 변경 무시
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	// BT 실행 (블랙보드 생성 포함)
	void StartBehaviorTree();

	// 감지 갱신 → 블랙보드 목표
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// 목표 지정 + 목표 사망 태그 구독 (사망한 대상 제외)
	void SetTargetActor(AActor* NewTarget);

	// 목표 해제 + 구독 해제
	void ClearTargetActor();

	// 목표 Character.State.Dead 추가 시 해제
	void OnTargetDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// 목표 블랙보드 키
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKeyName = TEXT("TargetActor");

	// 사망 태그 구독 중인 목표 ASC
	TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;
};
