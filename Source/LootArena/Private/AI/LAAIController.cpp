#include "AI/LAAIController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "LAGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ALAAIController::ALAAIController()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent")));

	// 시야 값은 BP 자식 PerceptionComponent > Senses Config에서 조정
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2500.f;
	// 반각 45 = 전방 90도
	SightConfig->PeripheralVisionAngleDegrees = 45.f;
	// 적대만 감지
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
	PerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
}

FGenericTeamId ALAAIController::GetGenericTeamId() const
{
	// 폰이 없거나 팀 인터페이스가 없으면 NoTeam
	return FGenericTeamId::GetTeamIdentifier(GetPawn());
}

void ALAAIController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	UE_LOG(LogTemp, Warning, TEXT("SetGenericTeamId ignored, team comes from pawn: %s"), *GetName());
}

void ALAAIController::OnPossess(APawn* InPawn)
{
	// Super 안에서 폰 PossessedBy
	Super::OnPossess(InPawn);

	// 감지 등록 시 팀은 NoTeam (빙의 전) → 폰 팀으로 시야 쿼리 다시 생성
	if (PerceptionComponent != nullptr)
	{
		PerceptionComponent->RequestStimuliListenerUpdate();
	}

	StartBehaviorTree();
}

void ALAAIController::OnUnPossess()
{
	Super::OnUnPossess();

	// 목표 사망 구독 해제
	ClearTargetActor();
}

void ALAAIController::StartBehaviorTree()
{
	if (BehaviorTree == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BehaviorTree not set: %s"), *GetName());
		return;
	}

	RunBehaviorTree(BehaviorTree);
}

void ALAAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 놓친 경우 목표 유지
	if (Actor == nullptr || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent == nullptr)
	{
		return;
	}

	// 목표가 없을 때만 지정
	if (BlackboardComponent->GetValueAsObject(TargetActorKeyName) == nullptr)
	{
		SetTargetActor(Actor);
	}
}

void ALAAIController::SetTargetActor(AActor* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(NewTarget);

	// 시체도 시야에 감지됨 → 사망한 대상 제외
	if (ASC != nullptr && ASC->HasMatchingGameplayTag(LAGameplayTags::Character_State_Dead))
	{
		return;
	}

	BlackboardComponent->SetValueAsObject(TargetActorKeyName, NewTarget);

	if (ASC != nullptr)
	{
		ASC->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::OnTargetDeadTagChanged);
		TargetAbilitySystemComponent = ASC;
	}
}

void ALAAIController::ClearTargetActor()
{
	// 이 컨트롤러의 구독 전부 해제
	UAbilitySystemComponent* ASC = TargetAbilitySystemComponent.Get();
	if (ASC != nullptr)
	{
		ASC->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}

	TargetAbilitySystemComponent.Reset();

	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent != nullptr)
	{
		BlackboardComponent->ClearValue(TargetActorKeyName);
	}
}

void ALAAIController::OnTargetDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 추가 시에만 (제거 무시)
	if (NewCount <= 0)
	{
		return;
	}

	ClearTargetActor();
}
