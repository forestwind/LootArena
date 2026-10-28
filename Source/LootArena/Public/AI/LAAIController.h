#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "LAAIController.generated.h"

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

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// 목표 블랙보드 키
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName TargetActorKeyName = TEXT("TargetActor");
};
