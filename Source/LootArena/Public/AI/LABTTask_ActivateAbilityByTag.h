#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "LABTTask_ActivateAbilityByTag.generated.h"

class UAbilitySystemComponent;
struct FAbilityEndedData;

UCLASS()
class LOOTARENA_API ULABTTask_ActivateAbilityByTag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULABTTask_ActivateAbilityByTag();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 대기 중인 어빌리티 종료 → 태스크 종료
	void OnAbilityEnded(const FAbilityEndedData& EndedData);

	// 종료 구독 해제
	void ClearAbilitySubscription();

	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;

	// 대기 중인 어빌리티
	FGameplayAbilitySpecHandle ActiveAbilityHandle;
	FDelegateHandle AbilityEndedDelegateHandle;
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<UBehaviorTreeComponent> OwnerBehaviorTree;
};
