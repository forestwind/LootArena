#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "LAInputConfig.generated.h"

class UInputAction;

UCLASS()
class LOOTARENA_API ULAInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	const UInputAction* FindNativeInputAction(const FGameplayTag& InputTag) const;

	// 이동·시점 입력
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<FGameplayTag, TObjectPtr<UInputAction>> InputTagToNativeAction;

	// 어빌리티 입력
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<FGameplayTag, TObjectPtr<UInputAction>> InputTagToAbilityAction;
};
