#include "AI/LABTTask_ActivateAbilityByTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "LAGameplayTags.h"

ULABTTask_ActivateAbilityByTag::ULABTTask_ActivateAbilityByTag()
{
	NodeName = TEXT("Activate Ability By Tag");
	AbilityTag = LAGameplayTags::Ability_Attack_Melee;

	// AI마다 인스턴스 (핸들·구독 보관)
	bCreateNodeInstance = true;
}

EBTNodeResult::Type ULABTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!AbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityTag not set: %s"), *GetName());
		return EBTNodeResult::Failed;
	}

	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AIController->GetPawn());
	if (ASC == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTag.GetSingleTagContainer(), MatchingSpecs);
	if (MatchingSpecs.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	// 발동 중 목록 변경 가능 → 포인터 대신 핸들 보관
	const FGameplayAbilitySpecHandle Handle = MatchingSpecs[0]->Handle;
	if (!ASC->TryActivateAbility(Handle))
	{
		return EBTNodeResult::Failed;
	}

	// 발동 중 이미 종료된 경우 대기 없음
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
	if (Spec == nullptr || !Spec->IsActive())
	{
		return EBTNodeResult::Succeeded;
	}

	ActiveAbilityHandle = Handle;
	AbilitySystemComponent = ASC;
	OwnerBehaviorTree = &OwnerComp;
	AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type ULABTTask_ActivateAbilityByTag::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	const FGameplayAbilitySpecHandle Handle = ActiveAbilityHandle;

	// 취소 콜백이 태스크를 다시 끝내지 않도록 구독 먼저 해제
	ClearAbilitySubscription();

	if (ASC != nullptr)
	{
		ASC->CancelAbilityHandle(Handle);
	}

	return EBTNodeResult::Aborted;
}

void ULABTTask_ActivateAbilityByTag::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
	if (EndedData.AbilitySpecHandle != ActiveAbilityHandle)
	{
		return;
	}

	UBehaviorTreeComponent* OwnerComp = OwnerBehaviorTree.Get();
	ClearAbilitySubscription();

	if (OwnerComp == nullptr)
	{
		return;
	}

	if (EndedData.bWasCancelled)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
	}
	else
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
}

void ULABTTask_ActivateAbilityByTag::ClearAbilitySubscription()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC != nullptr)
	{
		ASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
	}

	AbilityEndedDelegateHandle.Reset();
	ActiveAbilityHandle = FGameplayAbilitySpecHandle();
	AbilitySystemComponent.Reset();
	OwnerBehaviorTree.Reset();
}
