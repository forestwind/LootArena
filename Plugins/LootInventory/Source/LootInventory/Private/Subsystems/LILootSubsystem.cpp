// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/LILootSubsystem.h"
#include "Settings/LILootSettings.h"
#include "Drop/LIDropRow.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Pickup/LIItemPickup.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

// 이 파일 전용 로그 카테고리 (Output Log에서 LogLILoot로 표시)
DEFINE_LOG_CATEGORY_STATIC(LogLILoot, Log, All);

void ULILootSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Soft 참조 → 실제 테이블 로드
	UDataTable* DropTable = GetDefault<ULILootSettings>()->DropTable.LoadSynchronous();
	if (DropTable == nullptr)
	{
		UE_LOG(LogLILoot, Warning, TEXT("No drop table. Set it in Project Settings > Plugins > Loot Inventory > Drop Table"));
		return;
	}

	
	SetDropTable(DropTable);
}

void ULILootSubsystem::SetDropTable(UDataTable* InDropTable)
{
	
	DropGroupIdToRows.Reset();
	LoadedDropTable = InDropTable;

	if (InDropTable == nullptr)
	{
		return;
	}

	// 모든 행을 FLIDropRow로 읽음
	TArray<FLIDropRow*> Rows;
	InDropTable->GetAllRows<FLIDropRow>(TEXT("ULILootSubsystem::SetDropTable"), Rows);

	// DropGroupId 별로 묶음
	for (const FLIDropRow* Row : Rows)
	{
		DropGroupIdToRows.FindOrAdd(Row->DropGroupId).Add(Row);
	}

	UE_LOG(LogLILoot, Log, TEXT("%d drop groups loaded"), DropGroupIdToRows.Num());
}

TArray<FLIInventoryEntry> ULILootSubsystem::RollDrops(int32 InDropGroupId) const
{
	TArray<FLIInventoryEntry> Results;
	
	const TArray<const FLIDropRow*>* Rows = DropGroupIdToRows.Find(InDropGroupId);
	if (Rows == nullptr)
	{
		UE_LOG(LogLILoot, Warning, TEXT("RollDrops: unknown DropGroupId %d"), InDropGroupId);
		return Results;
	}

	// 줄마다 독립 굴림 → 0개도, 여러 개도 나올 수 있음
	for (const FLIDropRow* Row : *Rows)
	{
		// 확률 계산
		const int32 Roll = FMath::RandRange(1, 10000);
		if (Roll > Row->ChancePermyriad)
		{
			continue;
		}

		const int32 MaxCount = FMath::Max(Row->MinCount, Row->MaxCount);
		const int32 Count = FMath::RandRange(Row->MinCount, MaxCount);

		// MinCount가 0인 줄은 당첨돼도 0개일 수 있음 → 결과에 넣지 않음
		if (Count <= 0)
		{
			continue;
		}

		FLIInventoryEntry& Result = Results.AddDefaulted_GetRef();
		Result.ItemId = Row->ItemId;
		Result.Count = Count;
	}

	return Results;
}

void ULILootSubsystem::DropLoot(UWorld* InWorld, int32 InDropGroupId, const FVector& InLocation)
{
	if (InWorld == nullptr)
	{
		UE_LOG(LogLILoot, Warning, TEXT("DropLoot: no world (DropGroupId %d)"), InDropGroupId);
		return;
	}

	// Settings의 픽업 BP 클래스 로드
	const ULILootSettings* Settings = GetDefault<ULILootSettings>();
	UClass* PickupClass = Settings->PickupClass.LoadSynchronous();
	if (PickupClass == nullptr)
	{
		UE_LOG(LogLILoot, Warning, TEXT("No pickup class. Set it in Project Settings > Plugins > Loot Inventory > Pickup Class"));
		return;
	}

	// 확률 계산
	const TArray<FLIInventoryEntry> Results = RollDrops(InDropGroupId);

	//결과마다 픽업 아이템 액터 생성
	const float ScatterRadius = Settings->DropScatterRadius;
	for (const FLIInventoryEntry& Result : Results)
	{
		// 위치: 반경 안의 랜덤 지점만큼 옆으로 이동 (높이는 그대로, 픽업끼리 겹칠 수 있음)
		const FVector2D Offset = FMath::RandPointInCircle(ScatterRadius);
		const FTransform SpawnTransform(InLocation + FVector(Offset.X, Offset.Y, 0.f));

		// SpawnActorDeferred: 생성만 하고 BeginPlay는 FinishSpawning까지 미룸 → 값을 먼저 넣을 수 있음
		// AlwaysSpawn: 그 자리에 다른 물체가 있어도 생성
		ALIItemPickup* Pickup = InWorld->SpawnActorDeferred<ALIItemPickup>(PickupClass, SpawnTransform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Pickup == nullptr)
		{
			continue;
		}

		
		Pickup->SetItem(Result.ItemId, Result.Count);
		
		Pickup->FinishSpawning(SpawnTransform);
	}

	UE_LOG(LogLILoot, Log, TEXT("DropLoot %d: %d pickups"), InDropGroupId, Results.Num());
}


#if !UE_BUILD_SHIPPING
namespace LILootCommands
{
	// LI.RollDrop <DropGroupId>
	static void RollDrop(const TArray<FString>& InArgs, UWorld* InWorld)
	{
		ULILootSubsystem* Loot = nullptr;
		if (InWorld != nullptr && InWorld->GetGameInstance() != nullptr)
		{
			Loot = InWorld->GetGameInstance()->GetSubsystem<ULILootSubsystem>();
		}

		if (Loot == nullptr || InArgs.Num() < 1)
		{
			UE_LOG(LogLILoot, Warning, TEXT("Usage (during play): LI.RollDrop <DropGroupId>"));
			return;
		}

		// 인자는 문자열 → 숫자로 변환
		const int32 DropGroupId = FCString::Atoi(*InArgs[0]);
		const TArray<FLIInventoryEntry> Results = Loot->RollDrops(DropGroupId);

		UE_LOG(LogLILoot, Log, TEXT("RollDrop %d: %d results"), DropGroupId, Results.Num());
		for (const FLIInventoryEntry& Result : Results)
		{
			UE_LOG(LogLILoot, Log, TEXT("  ItemId %d x%d"), Result.ItemId, Result.Count);
		}
	}

	// 콘솔 명령 등록 (전역 변수 → 모듈이 로드될 때 자동 등록)
	static FAutoConsoleCommandWithWorldAndArgs RollDropCommand(
		TEXT("LI.RollDrop"),
		TEXT("Roll drop group. LI.RollDrop <DropGroupId>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&RollDrop));

	// LI.DropLoot <DropGroupId>: 플레이어 앞 200 위치에 드롭 (적 연결 전 테스트용)
	static void DropLoot(const TArray<FString>& InArgs, UWorld* InWorld)
	{
		ULILootSubsystem* Loot = nullptr;
		if (InWorld != nullptr && InWorld->GetGameInstance() != nullptr)
		{
			Loot = InWorld->GetGameInstance()->GetSubsystem<ULILootSubsystem>();
		}

		// 0번(첫 번째) 플레이어가 조종하는 폰 (플레이 중이 아니면 nullptr)
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(InWorld, 0);

		if (Loot == nullptr || PlayerPawn == nullptr || InArgs.Num() < 1)
		{
			UE_LOG(LogLILoot, Warning, TEXT("Usage (during play): LI.DropLoot <DropGroupId>"));
			return;
		}

		// 플레이어가 보는 방향으로 400 앞 (흩뿌림 반경 + 줍는 범위를 넘겨서 생성 직후 바로 줍지 않게)
		const FVector DropLocation = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * 400.f;

		// 콘솔이 넘겨준 월드에 드롭 (실제 게임에서는 죽은 적의 GetWorld())
		Loot->DropLoot(InWorld, FCString::Atoi(*InArgs[0]), DropLocation);
	}

	static FAutoConsoleCommandWithWorldAndArgs DropLootCommand(
		TEXT("LI.DropLoot"),
		TEXT("Drop loot in front of player. LI.DropLoot <DropGroupId>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DropLoot));
}
#endif
