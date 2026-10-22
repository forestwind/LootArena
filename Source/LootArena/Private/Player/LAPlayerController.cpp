#include "Player/LAPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/LAInputConfig.h"
#include "InputMappingContext.h"
#include "LAGameplayTags.h"
#include "LootInventory/LALILootToastWidget.h"
#include "UI/LAAttributeWidget.h"

void ALAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 입력·UI 사용
	if (!IsLocalController())
	{
		return;
	}

	AddDefaultMappingContext();
	CreateHUDWidgets();
}

void ALAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input Component required: %s"), *GetName());
		return;
	}

	if (UIInputConfig == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UIInputConfig not set: %s"), *GetName());
		return;
	}

	const UInputAction* InventoryAction = UIInputConfig->FindNativeInputAction(LAGameplayTags::InputTag_Inventory);
	if (InventoryAction != nullptr)
	{
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ThisClass::ToggleInventory);
	}
}

void ALAPlayerController::OnPossess(APawn* InPawn)
{
	// Super 안에서 폰 PossessedBy → 캐릭터 데이터(초기 스탯) 적용
	Super::OnPossess(InPawn);

	// 위젯 생성 전 빙의면 CreateHUDWidgets에서 연결
	if (HealthBarWidget != nullptr)
	{
		HealthBarWidget->SetTargetActor(InPawn);
	}
}

void ALAPlayerController::AddDefaultMappingContext()
{
	if (DefaultMappingContext == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext not set: %s"), *GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSubsystem == nullptr)
	{
		return;
	}

	InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ALAPlayerController::CreateHUDWidgets()
{
	if (LootToastWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootToastWidgetClass not set: %s"), *GetName());
	}
	else
	{
		LootToastWidget = CreateWidget<ULALILootToastWidget>(this, LootToastWidgetClass);
		if (LootToastWidget != nullptr)
		{
			LootToastWidget->AddToViewport();
		}
	}

	if (HealthBarWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthBarWidgetClass not set: %s"), *GetName());
	}
	else
	{
		HealthBarWidget = CreateWidget<ULAAttributeWidget>(this, HealthBarWidgetClass);
		if (HealthBarWidget != nullptr)
		{
			HealthBarWidget->AddToViewport();
			// 빙의가 먼저 끝난 경우 (nullptr이면 OnPossess에서 연결)
			HealthBarWidget->SetTargetActor(GetPawn());
		}
	}

	if (InventoryWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidgetClass not set: %s"), *GetName());
	}
	else
	{
		// 닫힌 상태로 생성 (닫혀 있어도 목록 갱신)
		InventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
		if (InventoryWidget != nullptr)
		{
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			InventoryWidget->AddToViewport();
		}
	}
}

void ALAPlayerController::ToggleInventory()
{
	if (InventoryWidget == nullptr)
	{
		return;
	}

	if (InventoryWidget->IsVisible())
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}
}

void ALAPlayerController::UseInventorySlot(int32 SlotId)
{
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = LAGameplayTags::Event_Item_Use;
	Payload.Instigator = ControlledPawn;
	// EventMagnitude = SlotId (정수를 float으로 변환 / 16,777,216까지 정확)
	Payload.EventMagnitude = static_cast<float>(SlotId);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(ControlledPawn, LAGameplayTags::Event_Item_Use, Payload);
}

void ALAPlayerController::OpenInventory()
{
	InventoryWidget->SetVisibility(ESlateVisibility::Visible);

	// 포커스 위젯 없음 (I 키가 게임 입력으로 계속 들어옴)
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ALAPlayerController::CloseInventory()
{
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
