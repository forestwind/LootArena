// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/LIInventorySubsystem.h"
#include "Subsystems/LIItemSubsystem.h"
#include "Data/LIItemRowBase.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"

// 이 파일 전용 로그 카테고리 (Output Log에서 LogLIInventory로 표시)
DEFINE_LOG_CATEGORY_STATIC(LogLIInventory, Log, All);

bool ULIInventorySubsystem::AddItem(int32 InItemId, int32 InCount)
{
	if (InCount <= 0)
	{
		UE_LOG(LogLIInventory, Warning, TEXT("AddItem: invalid count %d (ItemId %d)"), InCount, InItemId);
		return false;
	}

	// 데이터에 없는 아이템은 추가하지 않음 (드롭 테이블 오타 등)
	const ULIItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<ULIItemSubsystem>();
	const FLIItemRowBase* ItemRow = ItemSubsystem->FindItemRow(InItemId);
	if (ItemRow == nullptr)
	{
		UE_LOG(LogLIInventory, Warning, TEXT("AddItem: unknown ItemId %d"), InItemId);
		return false;
	}

	// MaxStack 0 이하 → 1로 취급 (0이면 아래 새 칸 반복에서 Remaining이 줄지 않아 무한 반복)
	const int32 MaxStack = FMath::Max(ItemRow->MaxStack, 1);
	int32 Remaining = InCount;

	// 1. 같은 아이템 칸 중 덜 찬 칸부터 채움
	for (FLIInventoryEntry& Entry : Entries)
	{
		if (Remaining == 0)
		{
			break;
		}

		// 다른 아이템이거나 꽉 찬 칸은 건너뜀 (데이터 교체로 MaxStack이 줄면 Space가 음수일 수 있음)
		const int32 Space = MaxStack - Entry.Count;
		if (Entry.ItemId != InItemId || Space <= 0)
		{
			continue;
		}

		const int32 AddCount = FMath::Min(Space, Remaining);
		Entry.Count += AddCount;
		Remaining -= AddCount;
	}

	// 2. 남은 개수는 새 칸으로 (한 칸에 MaxStack까지)
	while (Remaining > 0)
	{
		const int32 NewSlotId = FindFreeSlotId();

		// 새 칸은 뒤에 추가
		FLIInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.SlotId = NewSlotId;
		NewEntry.ItemId = InItemId;
		NewEntry.Count = FMath::Min(MaxStack, Remaining);
		Remaining -= NewEntry.Count;
	}

	UE_LOG(LogLIInventory, Log, TEXT("Added ItemId %d x%d (slots: %d)"), InItemId, InCount, Entries.Num());

	OnItemAdded.Broadcast(InItemId, InCount);
	OnInventoryChanged.Broadcast();
	return true;
}

bool ULIInventorySubsystem::RemoveItemFromSlot(int32 InSlotId)
{
	int32 FoundIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (Entries[Index].SlotId == InSlotId)
		{
			FoundIndex = Index;
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		UE_LOG(LogLIInventory, Warning, TEXT("RemoveItemFromSlot: unknown SlotId %d"), InSlotId);
		return false;
	}

	FLIInventoryEntry& Entry = Entries[FoundIndex];
	const int32 RemovedItemId = Entry.ItemId;
	Entry.Count -= 1;

	// 0개면 칸 삭제
	if (Entry.Count <= 0)
	{
		Entries.RemoveAt(FoundIndex);
	}

	UE_LOG(LogLIInventory, Log, TEXT("Removed ItemId %d from SlotId %d (slots: %d)"), RemovedItemId, InSlotId, Entries.Num());

	OnInventoryChanged.Broadcast();
	return true;
}

int32 ULIInventorySubsystem::FindFreeSlotId() const
{
	TSet<int32> UsedSlotIds;
	for (const FLIInventoryEntry& Entry : Entries)
	{
		UsedSlotIds.Add(Entry.SlotId);
	}

	// 1부터 빈 번호 찾기
	int32 SlotId = 1;
	while (UsedSlotIds.Contains(SlotId))
	{
		++SlotId;
	}

	return SlotId;
}

// 테스트용 콘솔 명령 (Shipping 빌드에서는 코드가 빠짐)
#if !UE_BUILD_SHIPPING
namespace LIInventoryCommands
{
	// 플레이 중이 아니면 GameInstance가 없어 nullptr
	static ULIInventorySubsystem* FindInventory(UWorld* InWorld)
	{
		if (InWorld == nullptr || InWorld->GetGameInstance() == nullptr)
		{
			return nullptr;
		}

		return InWorld->GetGameInstance()->GetSubsystem<ULIInventorySubsystem>();
	}

	// LI.AddItem <ItemId> [Count]
	static void AddItem(const TArray<FString>& InArgs, UWorld* InWorld)
	{
		ULIInventorySubsystem* Inventory = FindInventory(InWorld);
		if (Inventory == nullptr || InArgs.Num() < 1)
		{
			UE_LOG(LogLIInventory, Warning, TEXT("Usage (during play): LI.AddItem <ItemId> [Count]"));
			return;
		}
		
		const int32 ItemId = FCString::Atoi(*InArgs[0]);

		// Count 생략 시 1개
		int32 Count = 1;
		if (InArgs.Num() >= 2)
		{
			Count = FCString::Atoi(*InArgs[1]);
		}

		Inventory->AddItem(ItemId, Count);
	}

	// LI.PrintInventory
	static void PrintInventory(UWorld* InWorld)
	{
		ULIInventorySubsystem* Inventory = FindInventory(InWorld);
		if (Inventory == nullptr)
		{
			UE_LOG(LogLIInventory, Warning, TEXT("Usage (during play): LI.PrintInventory"));
			return;
		}

		const TArray<FLIInventoryEntry>& Entries = Inventory->GetEntries();
		UE_LOG(LogLIInventory, Log, TEXT("Inventory: %d slots"), Entries.Num());

		for (int32 Index = 0; Index < Entries.Num(); ++Index)
		{
			UE_LOG(LogLIInventory, Log, TEXT("  [%d] ItemId %d x%d"), Index, Entries[Index].ItemId, Entries[Index].Count);
		}
	}

	// 콘솔 명령 등록
	static FAutoConsoleCommandWithWorldAndArgs AddItemCommand(
		TEXT("LI.AddItem"),
		TEXT("Add item to inventory. LI.AddItem <ItemId> [Count]"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&AddItem));

	static FAutoConsoleCommandWithWorld PrintInventoryCommand(
		TEXT("LI.PrintInventory"),
		TEXT("Print inventory entries"),
		FConsoleCommandWithWorldDelegate::CreateStatic(&PrintInventory));
}
#endif
