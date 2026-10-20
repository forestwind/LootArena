// Fill out your copyright notice in the Description page of Project Settings.

#include "Viewer/LIItemViewerLibrary.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Data/LIItemDataAsset.h"
#include "UObject/UnrealType.h"
#include "UObject/TextProperty.h"

namespace LIItemViewer
{
	constexpr const TCHAR* ContextString = TEXT("LIItemViewer");

	// 프로퍼티 값 → 표시용 문자열
	// FText: 문자열 / Soft 참조: 에셋 이름 / GameplayTag: 태그 이름 / 그 외: ExportText
	FString ValueToString(const FProperty* Prop, const void* ValuePtr)
	{
		if (const FTextProperty* TextProp = CastField<FTextProperty>(Prop))
		{
			return TextProp->GetPropertyValue(ValuePtr).ToString();
		}
		if (const FSoftObjectProperty* SoftProp = CastField<FSoftObjectProperty>(Prop)) // Soft Class 포함
		{
			const FString AssetName = SoftProp->GetPropertyValue(ValuePtr).ToSoftObjectPath().GetAssetName();
			return AssetName.IsEmpty() ? TEXT("None") : AssetName;
		}
		const FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp && StructProp->Struct == FGameplayTag::StaticStruct())
		{
			return static_cast<const FGameplayTag*>(ValuePtr)->ToString();
		}

		FString Out;
		Prop->ExportTextItem_Direct(Out, ValuePtr, nullptr, nullptr, PPF_None);
		return Out;
	}

	// 상속 사슬 (기반 → 자식, StopStruct 제외)
	// 예) FPBLIConsumeItemRow → [FLIItemRowBase, FPBLIConsumeItemRow]
	TArray<const UStruct*> GetStructChain(const UStruct* Struct, const UStruct* StopStruct)
	{
		TArray<const UStruct*> Chain;
		for (const UStruct* Current = Struct; Current && Current != StopStruct; Current = Current->GetSuperStruct())
		{
			Chain.Insert(Current, 0);
		}
		return Chain;
	}

	// FLIItemAssetsBase::Icon 여부 (필드 목록 제외)
	bool IsAssetsIconProperty(const FProperty* Prop)
	{
		return Prop->GetOwnerStruct() == FLIItemAssetsBase::StaticStruct()
			&& Prop->GetFName() == GET_MEMBER_NAME_CHECKED(FLIItemAssetsBase, Icon);
	}

	// 필드 수집 (기반 → 자식, Data 없으면 이름만)
	// 구조체별 자체 선언 필드만 순회 (ExcludeSuper)
	void CollectFields(const UStruct* Struct, const UStruct* StopStruct, const void* Data,
	                   TArray<FLIItemViewerField>& OutFields)
	{
		for (const UStruct* Current : GetStructChain(Struct, StopStruct))
		{
			for (TFieldIterator<FProperty> It(Current, EFieldIteratorFlags::ExcludeSuper); It; ++It)
			{
				const FProperty* Prop = *It;
				if (IsAssetsIconProperty(Prop))
				{
					continue;
				}

				FLIItemViewerField& Field = OutFields.AddDefaulted_GetRef();
				Field.Name = Prop->GetAuthoredName();
				if (Data)
				{
					Field.Value = ValueToString(Prop, Prop->ContainerPtrToValuePtr<void>(Data));
				}
			}
		}
	}

	TArray<FString> CollectFieldNames(const UStruct* Struct, const UStruct* StopStruct)
	{
		TArray<FLIItemViewerField> Fields;
		CollectFields(Struct, StopStruct, nullptr, Fields);

		TArray<FString> Names;
		for (const FLIItemViewerField& Field : Fields)
		{
			Names.Add(Field.Name);
		}
		return Names;
	}

	// 테이블 전체 행 (기반 구조체 포인터)
	TArray<FLIItemRowBase*> GetItemRows(const UDataTable* Table)
	{
		TArray<FLIItemRowBase*> Rows;
		Table->GetAllRows<FLIItemRowBase>(ContextString, Rows);
		return Rows;
	}

	// ItemId 일치 행 (선형 탐색)
	const FLIItemRowBase* FindItemRow(const UDataTable* Table, int32 ItemId)
	{
		for (const FLIItemRowBase* Row : GetItemRows(Table))
		{
			if (Row->ItemId == ItemId)
			{
				return Row;
			}
		}
		return nullptr;
	}

	// 리소스 맵 값 구조체 타입 (TMap<int32, FLIItemAssetsBase 파생>)
	const UScriptStruct* FindAssetsStruct(const ULIItemDataAssetBase* DataAsset)
	{
		for (TFieldIterator<FMapProperty> It(DataAsset->GetClass()); It; ++It)
		{
			const FStructProperty* ValueProp = CastField<FStructProperty>(It->ValueProp);
			if (It->KeyProp->IsA<FIntProperty>() && ValueProp && ValueProp->Struct->IsChildOf(FLIItemAssetsBase::StaticStruct()))
			{
				return ValueProp->Struct;
			}
		}
		return nullptr;
	}
}

bool ULIItemViewerLibrary::IsItemTable(const UDataTable* Table)
{
	return Table && Table->GetRowStruct() && Table->GetRowStruct()->IsChildOf(FLIItemRowBase::StaticStruct());
}

TArray<int32> ULIItemViewerLibrary::GetTableItemIds(const UDataTable* Table)
{
	TArray<int32> ItemIds;
	if (IsItemTable(Table))
	{
		for (const FLIItemRowBase* Row : LIItemViewer::GetItemRows(Table))
		{
			ItemIds.Add(Row->ItemId);
		}
	}
	return ItemIds;
}

TArray<FString> ULIItemViewerLibrary::GetTableFieldNames(const UDataTable* Table)
{
	if (!IsItemTable(Table))
	{
		return {};
	}
	return LIItemViewer::CollectFieldNames(Table->GetRowStruct(), FTableRowBase::StaticStruct());
}

bool ULIItemViewerLibrary::GetItemRowFields(const UDataTable* Table, int32 ItemId, TArray<FLIItemViewerField>& OutFields)
{
	OutFields.Reset();
	if (!IsItemTable(Table))
	{
		return false;
	}

	const FLIItemRowBase* Row = LIItemViewer::FindItemRow(Table, ItemId);
	if (!Row)
	{
		return false;
	}

	LIItemViewer::CollectFields(Table->GetRowStruct(), FTableRowBase::StaticStruct(), Row, OutFields);
	return true;
}

TArray<FString> ULIItemViewerLibrary::GetAssetFieldNames(const ULIItemDataAssetBase* DataAsset)
{
	const UScriptStruct* AssetsStruct = DataAsset ? LIItemViewer::FindAssetsStruct(DataAsset) : nullptr;
	if (!AssetsStruct)
	{
		return {};
	}
	return LIItemViewer::CollectFieldNames(AssetsStruct, nullptr);
}

bool ULIItemViewerLibrary::GetItemAssetFields(const ULIItemDataAssetBase* DataAsset, int32 ItemId,
                                              TSoftObjectPtr<UTexture2D>& OutIcon, TArray<FLIItemViewerField>& OutFields)
{
	OutIcon.Reset();
	OutFields.Reset();

	const UScriptStruct* AssetsStruct = DataAsset ? LIItemViewer::FindAssetsStruct(DataAsset) : nullptr;
	const FLIItemAssetsBase* Assets = AssetsStruct ? DataAsset->FindAssets(ItemId) : nullptr;
	if (!Assets)
	{
		return false;
	}

	OutIcon = Assets->Icon;
	LIItemViewer::CollectFields(AssetsStruct, nullptr, Assets, OutFields);
	return true;
}
