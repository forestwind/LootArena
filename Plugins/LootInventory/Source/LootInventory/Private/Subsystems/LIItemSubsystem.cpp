// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/LIItemSubsystem.h"
#include "Settings/LILootSettings.h"
#include "Data/LIItemDataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"

#if WITH_EDITOR
#include "Settings/ProjectPackagingSettings.h"
#endif

// 이 파일 전용 로그 카테고리 (Output Log에서 LogLIItem으로 표시)
DEFINE_LOG_CATEGORY_STATIC(LogLIItem, Log, All);

void ULIItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 교체할 테이블 리스트(기본은 ULILootSettings에서 로드된 테이블)
	const TMap<FGameplayTag, UDataTable*> ChangeTables;
	ReloadItemData(ChangeTables);

#if WITH_EDITOR
	// 로드가 끝난 뒤 검사 (LoadedItemDataAssets가 채워진 상태여야 함)
	CheckCookDirectories();
#endif
}

void ULIItemSubsystem::ReloadItemData(const TMap<FGameplayTag, UDataTable*>& InChangeTables)
{
	// 이전 데이터 전부 비움
	ItemIdToEntry.Reset();
	LoadedItemDataAssets.Reset();
	LoadedItemTables.Reset();

	const ULILootSettings* Settings = GetDefault<ULILootSettings>();

	// 미등록
	if (Settings->ItemDataAssets.IsEmpty())
	{
		UE_LOG(LogLIItem, Warning, TEXT("No item data assets. Register them in Project Settings > Plugins > Loot Inventory > Item Data Assets"));
		return;
	}

	for (const TSoftObjectPtr<ULIItemDataAssetBase>& ItemDataAssetPath : Settings->ItemDataAssets)
	{
		// Soft 참조 → 실제 에셋 로드
		ULIItemDataAssetBase* ItemDataAsset = ItemDataAssetPath.LoadSynchronous();
		if (ItemDataAsset == nullptr)
		{
			UE_LOG(LogLIItem, Warning, TEXT("Failed to load item data asset '%s'. Check Project Settings > Plugins > Loot Inventory > Item Data Assets"), *ItemDataAssetPath.ToString());
			continue;
		}

		UDataTable* ItemTable = ItemDataAsset->ItemTable.LoadSynchronous();
		if (ItemTable == nullptr)
		{
			UE_LOG(LogLIItem, Warning, TEXT("'%s' has no Item Table"), *ItemDataAsset->GetName());
			continue;
		}

		// 교체 테이블이 있으면 그 테이블, 없으면 로컬 테이블
		UDataTable* const* ChangeTable = InChangeTables.Find(ItemDataAsset->ItemType);
		if (ChangeTable != nullptr && *ChangeTable != nullptr)
		{
			ItemTable = *ChangeTable;
		}

		LoadedItemDataAssets.Add(ItemDataAsset);
		LoadedItemTables.Add(ItemTable);
		AddTableRows(ItemTable, ItemDataAsset);
	}

	UE_LOG(LogLIItem, Log, TEXT("%d items loaded"), ItemIdToEntry.Num());
}

void ULIItemSubsystem::AddTableRows(UDataTable* InItemTable, const ULIItemDataAssetBase* InItemDataAsset)
{
	// 행 구조체가 무엇이든 기반(FLIItemRowBase)으로 읽음 → 타입을 몰라도 됨
	// 아이템 테이블이 아니면 엔진이 에러 로그를 남기고 빈 배열 반환
	TArray<FLIItemRowBase*> Rows;
	InItemTable->GetAllRows<FLIItemRowBase>(TEXT("ULIItemSubsystem::AddTableRows"), Rows);

	// 묶음의 ItemType과 행의 GetItemType() 비교 (한 테이블은 같은 행 구조체라 첫 행만 확인)
	if (Rows.Num() > 0 && Rows[0]->GetItemType() != InItemDataAsset->ItemType)
	{
		UE_LOG(LogLIItem, Warning, TEXT("'%s' Item Type (%s) differs from row type (%s)"),
			*InItemDataAsset->GetName(), *InItemDataAsset->ItemType.ToString(), *Rows[0]->GetItemType().ToString());
	}

	for (const FLIItemRowBase* Row : Rows)
	{
		// ItemId 중복 → 경고 후 나중 것으로 덮어씀
		const FLIItemEntry* PreviousEntry = ItemIdToEntry.Find(Row->ItemId);
		if (PreviousEntry != nullptr)
		{
			UE_LOG(LogLIItem, Warning, TEXT("Duplicate ItemId %d: '%s' overwrites '%s'"),
				Row->ItemId, *InItemDataAsset->GetName(), *PreviousEntry->ItemDataAsset->GetName());
		}

		FLIItemEntry& Entry = ItemIdToEntry.Add(Row->ItemId);
		Entry.Row = Row;
		Entry.ItemDataAsset = InItemDataAsset;
	}
}

const FLIItemRowBase* ULIItemSubsystem::FindItemRow(int32 InItemId) const
{
	const FLIItemEntry* Entry = ItemIdToEntry.Find(InItemId);
	if (Entry == nullptr)
	{
		return nullptr;
	}

	return Entry->Row;
}

const FLIItemAssetsBase* ULIItemSubsystem::FindItemAssets(int32 InItemId) const
{
	const FLIItemEntry* Entry = ItemIdToEntry.Find(InItemId);
	if (Entry == nullptr)
	{
		return nullptr;
	}

	return Entry->ItemDataAsset->FindAssets(InItemId);
}

bool ULIItemSubsystem::GetItemDisplayInfo(int32 InItemId, FLIItemDisplayInfo& OutInfo) const
{
	// 이전 값이 남지 않게 비우고 시작
	OutInfo = FLIItemDisplayInfo();

	const FLIItemEntry* Entry = ItemIdToEntry.Find(InItemId);
	if (Entry == nullptr)
	{
		return false;
	}
	
	const FLIItemRowBase* Row = Entry->Row;
	OutInfo.ItemId = Row->ItemId;
	OutInfo.DisplayName = Row->DisplayName;
	OutInfo.Description = Row->Description;
	OutInfo.Rarity = Row->Rarity;
	OutInfo.ItemType = Row->GetItemType();
	OutInfo.MaxStack = Row->MaxStack;

	// 아이콘 로드
	const FLIItemAssetsBase* Assets = Entry->ItemDataAsset->FindAssets(InItemId);
	if (Assets != nullptr)
	{
		OutInfo.Icon = Assets->Icon.LoadSynchronous();
	}

	return true;
}

#if WITH_EDITOR
void ULIItemSubsystem::CheckCookDirectories() const
{
	// 등록된 묶음과 아이템 테이블이 있는 폴더 모으기
	TSet<FString> DataFolders;
	for (const ULIItemDataAssetBase* ItemDataAsset : LoadedItemDataAssets)
	{
		// GetLongPackagePath: 에셋 경로에서 폴더만 분리
		DataFolders.Add(FPackageName::GetLongPackagePath(ItemDataAsset->GetPackage()->GetName()));

		// 실제 로드한 테이블이 아니라 묶음에 지정된 원래 테이블 경로 (교체 테이블은 런타임 생성이라 폴더 없음)
		DataFolders.Add(FPackageName::GetLongPackagePath(ItemDataAsset->ItemTable.GetLongPackageName()));
	}

	// 드롭 테이블 폴더 (Settings에 지정된 경우만)
	const ULILootSettings* Settings = GetDefault<ULILootSettings>();
	if (!Settings->DropTable.IsNull())
	{
		DataFolders.Add(FPackageName::GetLongPackagePath(Settings->DropTable.GetLongPackageName()));
	}

	// 픽업 BP 폴더 (C++ 클래스는 /Script/ 경로라 쿡 대상 아님)
	if (!Settings->PickupClass.IsNull())
	{
		const FString PickupPackage = Settings->PickupClass.GetLongPackageName();
		if (!PickupPackage.StartsWith(TEXT("/Script/")))
		{
			DataFolders.Add(FPackageName::GetLongPackagePath(PickupPackage));
		}
	}

	const UProjectPackagingSettings* PackagingSettings = GetDefault<UProjectPackagingSettings>();

	for (const FString& DataFolder : DataFolders)
	{
		bool bInCookList = false;

		// DirectoriesToAlwaysCook = "Additional Asset Directories to Cook" 목록
		for (const FDirectoryPath& CookDirectory : PackagingSettings->DirectoriesToAlwaysCook)
		{
			// 목록은 "/Game/Features/..." 또는 "/Game" 기준 상대 경로 "Features/..."로 저장될 수 있음
			// 상대 경로면 "/Game/"을 붙여 형식을 맞춤
			FString CookPath = CookDirectory.Path;
			if (!CookPath.StartsWith(TEXT("/")))
			{
				CookPath = TEXT("/Game/") + CookPath;
			}

			// 같은 폴더이거나 상위 폴더가 등록돼 있으면 포함
			// "/" 붙여 비교 → "/Game/Feat"가 "/Game/Features"를 포함한다고 잘못 판단하지 않음
			if (DataFolder == CookPath || DataFolder.StartsWith(CookPath + TEXT("/")))
			{
				bInCookList = true;
				break;
			}
		}

		if (!bInCookList)
		{
			UE_LOG(LogLIItem, Warning, TEXT("'%s' is not in the cook list. Add it to Project Settings > Packaging > Additional Asset Directories to Cook"), *DataFolder);
		}
	}
}
#endif
