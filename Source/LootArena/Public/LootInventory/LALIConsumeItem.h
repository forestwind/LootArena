#pragma once

#include "CoreMinimal.h"
#include "Data/LIItemDataAsset.h"
#include "LALIConsumeItem.generated.h"

class UGameplayEffect;

// 소비 아이템 행 (CSV 컬럼)
USTRUCT(BlueprintType)
struct LOOTARENA_API FLALIConsumeItemRow : public FLIItemRowBase
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetItemType() const override;
};

// 소비 아이템 리소스
USTRUCT(BlueprintType)
struct LOOTARENA_API FLALIConsumeItemAssets : public FLIItemAssetsBase
{
	GENERATED_BODY()

	// 사용 시 자신에게 적용할 GE (사용 시점 로드)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSoftClassPtr<UGameplayEffect> UseEffect;
};

// 소비 아이템 묶음 (Key: ItemId)
UCLASS()
class LOOTARENA_API ULALIConsumeItemDataAsset : public ULIItemDataAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TMap<int32, FLALIConsumeItemAssets> Assets;

	virtual const FLIItemAssetsBase* FindAssets(int32 InItemId) const override { return Assets.Find(InItemId); }
};
