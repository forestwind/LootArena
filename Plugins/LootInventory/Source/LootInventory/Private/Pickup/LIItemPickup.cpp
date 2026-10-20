// Fill out your copyright notice in the Description page of Project Settings.

#include "Pickup/LIItemPickup.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Subsystems/LIInventorySubsystem.h"

ALIItemPickup::ALIItemPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(50.f);

	// OverlapAllDynamic: 막지 않고 겹침만 감지
	PickupSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(PickupSphere);
}

void ALIItemPickup::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에 직접 배치한 경우 디테일 패널 값으로 겉모습 갱신
	OnItemSet();

	// 대기 시간 뒤에 줍기 활성화
	if (PickupDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(PickupDelayTimer, this, &ALIItemPickup::EnablePickup, PickupDelay, false);
	}
	else
	{
		EnablePickup();
	}
}

void ALIItemPickup::SetItem(int32 InItemId, int32 InCount)
{
	ItemId = InItemId;
	Count = InCount;
}

void ALIItemPickup::EnablePickup()
{
	// 이제부터 구체 충돌처리 추가
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ALIItemPickup::HandleBeginOverlap);

	// 이미 겹쳐 있는 폰도 확인
	TArray<AActor*> OverlappingActors;
	PickupSphere->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
	for (AActor* OverlappingActor : OverlappingActors)
	{
		TryPickup(OverlappingActor);
	}
}

void ALIItemPickup::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                       int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryPickup(OtherActor);
}

void ALIItemPickup::TryPickup(AActor* InActor)
{
	// 이미 주웠으면 무시
	if (bPickedUp)
	{
		return;
	}

	// 플레이어가 조종하는 폰만
	const APawn* Pawn = Cast<APawn>(InActor);
	if (Pawn == nullptr || !Pawn->IsPlayerControlled())
	{
		return;
	}

	ULIInventorySubsystem* Inventory = GetGameInstance()->GetSubsystem<ULIInventorySubsystem>();
	if (!Inventory->AddItem(ItemId, Count))
	{
		// 아이템 추가 실패 (없는 ItemId 등)
		return;
	}

	bPickedUp = true;
	OnPickedUp();
	Destroy();
}