#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LAWeaponData.generated.h"

class UStaticMesh;
class ULAAbilitySet;

UCLASS()
class LOOTARENA_API ULAWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	TObjectPtr<UStaticMesh> Mesh;

	// 캐릭터 메시의 소켓 또는 본 이름
	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	FName AttachSocketName = TEXT("hand_socket_r");

	// 장착 시 부여, 해제 시 제거
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TObjectPtr<ULAAbilitySet> AbilitySet;
};
