// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LIItemPickup.generated.h"

class USphereComponent;

// 바닥 아이템 (플레이어가 닿으면 인벤토리에 추가 후 제거)
UCLASS()
class LOOTINVENTORY_API ALIItemPickup : public AActor
{
	GENERATED_BODY()

public:
	ALIItemPickup();

	// 아이템 지정 (생성 직후 BeginPlay 전에 호출, 모델은 BeginPlay에서 갱신)
	void SetItem(int32 InItemId, int32 InCount);

protected:
	virtual void BeginPlay() override;

	// 겉모습 갱신 (BP에서 구현: 아이콘, 등급 색 등)
	UFUNCTION(BlueprintImplementableEvent, Category = "LootInventory")
	void OnItemSet();

	// 획득 직전 연출 (BP에서 구현: 사운드, 이펙트)
	UFUNCTION(BlueprintImplementableEvent, Category = "LootInventory")
	void OnPickedUp();

	// 줍는 범위 (겹침만 감지, 막지 않음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LootInventory")
	TObjectPtr<USphereComponent> PickupSphere;

	// 레벨에 직접 배치할 때 디테일 패널에서 지정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	int32 Count = 1;

	// 생성 후 줍기까지 대기 시간 (초, 0이면 바로 줍기 가능)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootInventory")
	float PickupDelay = 0.5f;

private:
	// 대기 시간이 끝나면 줍기 활성화
	void EnablePickup();

	// 줍기 시도
	void TryPickup(AActor* InActor);

	// 겹침 이벤트 연결 함수
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 중복 추가 방지
	bool bPickedUp = false;

	// 줍기 대기 타이머
	FTimerHandle PickupDelayTimer;
};