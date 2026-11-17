#include "Game/LAGameMode.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AI/LAAIController.h"
#include "Character/LAEnemyCharacter.h"
#include "EngineUtils.h"
#include "Game/LAArenaData.h"
#include "Game/LAEnemySpawnPoint.h"
#include "Game/LAGameState.h"
#include "Kismet/GameplayStatics.h"
#include "LAGameplayTags.h"
#include "Subsystems/LIInventorySubsystem.h"

ALAGameMode::ALAGameMode()
{
	GameStateClass = ALAGameState::StaticClass();
}

void ALAGameMode::BeginPlay()
{
	Super::BeginPlay();

	StartArena();
}

void ALAGameMode::StartArena()
{
	if (ArenaData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ArenaData not set: %s"), *GetName());
		return;
	}

	ALAGameState* ArenaGameState = GetGameState<ALAGameState>();
	if (ArenaGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState is not LAGameState: %s"), *GetName());
		return;
	}

	ResetData();

	CollectSpawnPoints();
	TrackPlayer();

	ArenaGameState->SetStageCount(ArenaData->GetStageCount());
	SetArenaState(ELAArenaState::WaitingToStart);

	ScheduleNextStage(ArenaData->StartDelay);
}

void ALAGameMode::ResetData()
{
	// GameInstance 서브시스템이라 레벨을 다시 열어도 남음
	ULIInventorySubsystem* Inventory = GetGameInstance()->GetSubsystem<ULIInventorySubsystem>();
	if (Inventory != nullptr)
	{
		Inventory->RemoveAllItems();
	}
}

void ALAGameMode::CollectSpawnPoints()
{
	SpawnIdToPoint.Reset();

	for (TActorIterator<ALAEnemySpawnPoint> It(GetWorld()); It; ++It)
	{
		ALAEnemySpawnPoint* SpawnPoint = *It;
		const int32 SpawnId = SpawnPoint->GetSpawnId();

		// 0 이하 = 미입력
		if (SpawnId <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnId not set, ignored: %s"), *SpawnPoint->GetName());
		}
		else if (SpawnIdToPoint.Contains(SpawnId))
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplicate SpawnId %d, ignored: %s"), SpawnId, *SpawnPoint->GetName());
		}
		else
		{
			SpawnIdToPoint.Add(SpawnId, SpawnPoint);
		}
	}

	if (SpawnIdToPoint.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid LAEnemySpawnPoint in level"));
	}
}

void ALAGameMode::StartNextStage()
{
	CurrentStageIndex++;

	FLAArenaStage Stage;
	if (!ArenaData->GetStage(CurrentStageIndex, Stage))
	{
		UE_LOG(LogTemp, Warning, TEXT("Stage index %d not found in %s"), CurrentStageIndex, *ArenaData->GetName());
		return;
	}

	AliveEnemies.Reset();
	SetArenaState(ELAArenaState::InProgress);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	for (const FLAArenaSpawnEntry& Entry : Stage.SpawnEntries)
	{
		ALAEnemyCharacter* Enemy = SpawnEnemy(Entry, PlayerPawn);
		if (Enemy != nullptr)
		{
			TrackEnemy(Enemy);
		}
	}

	// 스테이지 번호는 1부터 표시
	UE_LOG(LogTemp, Log, TEXT("Stage %d started: %d / %d enemies spawned"), CurrentStageIndex + 1, AliveEnemies.Num(), Stage.SpawnEntries.Num());

	// 전부 스폰 실패 시 멈춤 방지
	CheckStageCleared();
}

ALAEnemyCharacter* ALAGameMode::SpawnEnemy(const FLAArenaSpawnEntry& Entry, APawn* PlayerPawn)
{
	if (Entry.EnemyClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyClass not set, SpawnId %d"), Entry.SpawnId);
		return nullptr;
	}

	ALAEnemySpawnPoint* SpawnPoint = SpawnIdToPoint.FindRef(Entry.SpawnId);
	if (SpawnPoint == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No spawn point for SpawnId %d"), Entry.SpawnId);
		return nullptr;
	}

	// 지점 위치 그대로
	const FVector SpawnLocation = SpawnPoint->GetActorLocation();

	// 플레이어 쪽 Yaw만, 플레이어 없으면 지점 방향
	FRotator SpawnRotation = SpawnPoint->GetActorRotation();
	if (PlayerPawn != nullptr)
	{
		const FVector ToPlayer = PlayerPawn->GetActorLocation() - SpawnLocation;
		SpawnRotation = FRotator(0.f, ToPlayer.Rotation().Yaw, 0.f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALAEnemyCharacter* Enemy = GetWorld()->SpawnActor<ALAEnemyCharacter>(Entry.EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Enemy == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn %s, SpawnId %d"), *Entry.EnemyClass->GetName(), Entry.SpawnId);
		return nullptr;
	}

	// SpawnActor 안에서 빙의·BT 시작 완료 → 블랙보드 있음
	ALAAIController* AIController = Cast<ALAAIController>(Enemy->GetController());
	if (AIController != nullptr && PlayerPawn != nullptr)
	{
		AIController->SetTargetActor(PlayerPawn);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Target not assigned: %s"), *Enemy->GetName());
	}

	return Enemy;
}

void ALAGameMode::TrackEnemy(ALAEnemyCharacter* Enemy)
{
	AliveEnemies.Add(Enemy);

	// 콜백에서 어느 적인지 구분 → 적을 함께 전달
	Enemy->GetAbilitySystemComponent()->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::OnEnemyDeadTagChanged, TWeakObjectPtr<ALAEnemyCharacter>(Enemy));
}

void ALAGameMode::OnEnemyDeadTagChanged(const FGameplayTag Tag, int32 NewCount, TWeakObjectPtr<ALAEnemyCharacter> Enemy)
{
	// 태그 추가 시에만 (제거 무시)
	if (NewCount <= 0)
	{
		return;
	}

	// 사망 태그 구독 해제
	ALAEnemyCharacter* DeadEnemy = Enemy.Get();
	if (DeadEnemy != nullptr)
	{
		DeadEnemy->GetAbilitySystemComponent()->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}

	// 목록에 없던 적이면 무시 (중복 집계 방지)
	if (AliveEnemies.Remove(Enemy) == 0)
	{
		return;
	}

	CheckStageCleared();
}

void ALAGameMode::CheckStageCleared()
{
	const ALAGameState* ArenaGameState = GetGameState<ALAGameState>();
	if (ArenaGameState == nullptr || ArenaGameState->GetArenaState() != ELAArenaState::InProgress)
	{
		return;
	}

	if (!AliveEnemies.IsEmpty())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stage %d cleared"), CurrentStageIndex + 1);

	// 마지막 스테이지 → 사이 대기 없이 승리
	if (CurrentStageIndex + 1 >= ArenaData->GetStageCount())
	{
		FinishArena(ELAArenaState::Victory);
		return;
	}

	SetArenaState(ELAArenaState::StageCleared);

	ScheduleNextStage(ArenaData->StageClearedDelay);
}

void ALAGameMode::TrackPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerPawn);
	if (ASC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player ASC not found, defeat disabled"));
		return;
	}

	ASC->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::OnPlayerDeadTagChanged);
	PlayerAbilitySystemComponent = ASC;
}

void ALAGameMode::OnPlayerDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 태그 추가 시에만 (제거 무시)
	if (NewCount <= 0)
	{
		return;
	}

	FinishArena(ELAArenaState::Defeat);
}

void ALAGameMode::FinishArena(ELAArenaState ResultState)
{
	const ALAGameState* ArenaGameState = GetGameState<ALAGameState>();
	if (ArenaGameState == nullptr)
	{
		return;
	}

	// 이미 종료 → 무시 (승리 후 사망 등)
	const ELAArenaState CurrentState = ArenaGameState->GetArenaState();
	if (CurrentState == ELAArenaState::Victory || CurrentState == ELAArenaState::Defeat)
	{
		return;
	}

	// 대기 중인 다음 스테이지 취소
	GetWorldTimerManager().ClearTimer(StageTimerHandle);

	// 플레이어 사망 태그 구독 해제
	UAbilitySystemComponent* ASC = PlayerAbilitySystemComponent.Get();
	if (ASC != nullptr)
	{
		ASC->RegisterGameplayTagEvent(LAGameplayTags::Character_State_Dead, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
	PlayerAbilitySystemComponent.Reset();

	SetArenaState(ResultState);

	UE_LOG(LogTemp, Log, TEXT("Arena finished: %s"), *UEnum::GetValueAsString(ResultState));
}

void ALAGameMode::ScheduleNextStage(float Delay)
{
	// 0 이하면 타이머가 설정되지 않음
	GetWorldTimerManager().SetTimer(StageTimerHandle, this, &ThisClass::StartNextStage, FMath::Max(Delay, 0.1f), false);
}

void ALAGameMode::SetArenaState(ELAArenaState NewState)
{
	ALAGameState* ArenaGameState = GetGameState<ALAGameState>();
	if (ArenaGameState == nullptr)
	{
		return;
	}

	// 시작 전(INDEX_NONE) = 0
	ArenaGameState->SetArenaState(NewState, CurrentStageIndex + 1);
}
