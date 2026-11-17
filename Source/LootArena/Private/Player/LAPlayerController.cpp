#include "Player/LAPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game/LAGameState.h"
#include "Input/LAInputConfig.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LAGameplayTags.h"
#include "LootInventory/LALILootToastWidget.h"
#include "UI/LAArenaStateWidget.h"
#include "UI/LAAttributeWidget.h"

void ALAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 입력·UI 사용
	if (IsLocalController())
	{
		// 이전 레벨 UI 전용 입력 해제 (뷰포트 설정은 레벨 이동 후에도 남음)
		SetGameInputMode();
		AddDefaultMappingContext();
		CreateHUDWidgets();
		ConnectGameState();
	}
}

void ALAPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisconnectGameState();
	GetWorldTimerManager().ClearTimer(ResultTimerHandle);

	Super::EndPlay(EndPlayReason);
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
	LootToastWidget = Cast<ULALILootToastWidget>(CreateViewportWidget(LootToastWidgetClass, TEXT("LootToastWidgetClass")));

	HealthBarWidget = Cast<ULAAttributeWidget>(CreateViewportWidget(HealthBarWidgetClass, TEXT("HealthBarWidgetClass")));
	if (HealthBarWidget != nullptr)
	{
		// 빙의가 먼저 끝난 경우 (nullptr이면 OnPossess에서 연결)
		HealthBarWidget->SetTargetActor(GetPawn());
	}

	ArenaStageWidget = Cast<ULAArenaStateWidget>(CreateViewportWidget(ArenaStageWidgetClass, TEXT("ArenaStageWidgetClass")));

	// 닫힌 상태로 생성 (닫혀 있어도 목록 갱신)
	InventoryWidget = CreateViewportWidget(InventoryWidgetClass, TEXT("InventoryWidgetClass"));
	if (InventoryWidget != nullptr)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UUserWidget* ALAPlayerController::CreateViewportWidget(TSubclassOf<UUserWidget> WidgetClass, const TCHAR* ClassPropertyName)
{
	if (WidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s not set: %s"), ClassPropertyName, *GetName());
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (Widget != nullptr)
	{
		Widget->AddToViewport();
	}

	return Widget;
}

void ALAPlayerController::ToggleInventory()
{
	// 아레나 종료 후 열기 차단
	if (InventoryWidget == nullptr || IsArenaFinished())
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

void ALAPlayerController::RestartArena()
{
	// 결과 화면 UI 전용 입력 해제 (뷰포트 설정은 레벨 이동 후에도 남음)
	SetGameInputMode();

	// 인벤토리 초기화는 GameMode 아레나 시작 시
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void ALAPlayerController::QuitArena()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
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

	SetGameInputMode();
}

void ALAPlayerController::SetGameInputMode()
{
	// 첫 클릭도 게임 입력으로 (프로젝트 기본 캡처 모드와 같음)
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
}

void ALAPlayerController::ConnectGameState()
{
	// 재호출 시 중복 구독 방지
	DisconnectGameState();

	ALAGameState* FoundGameState = GetWorld()->GetGameState<ALAGameState>();
	if (FoundGameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameState is not LAGameState: %s"), *GetName());
		return;
	}

	ConnectedGameState = FoundGameState;
	ArenaStateChangedHandle = FoundGameState->OnArenaStateChanged.AddUObject(this, &ThisClass::HandleArenaStateChanged);
}

void ALAPlayerController::DisconnectGameState()
{
	ALAGameState* OldGameState = ConnectedGameState.Get();
	if (OldGameState != nullptr)
	{
		OldGameState->OnArenaStateChanged.Remove(ArenaStateChangedHandle);
	}

	ConnectedGameState.Reset();
	ArenaStateChangedHandle.Reset();
}

void ALAPlayerController::HandleArenaStateChanged()
{
	if (!IsArenaFinished())
	{
		return;
	}

	// 결과 화면 예약·표시 후 재진입 무시
	if (ArenaResultWidget != nullptr || GetWorldTimerManager().IsTimerActive(ResultTimerHandle))
	{
		return;
	}

	if (InventoryWidget != nullptr && InventoryWidget->IsVisible())
	{
		CloseInventory();
	}

	// 0 이하면 바로 표시 (타이머는 0 이하면 설정되지 않음)
	if (ResultDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(ResultTimerHandle, this, &ThisClass::ShowArenaResult, ResultDelay, false);
	}
	else
	{
		ShowArenaResult();
	}
}

void ALAPlayerController::ShowArenaResult()
{
	ArenaResultWidget = Cast<ULAArenaStateWidget>(CreateViewportWidget(ArenaResultWidgetClass, TEXT("ArenaResultWidgetClass")));
	if (ArenaResultWidget == nullptr)
	{
		return;
	}

	// 눌린 키 떼기 처리 (UI 전용 입력 후에는 키 떼기가 게임에 전달되지 않음)
	FlushPressedKeys();

	// 게임 입력 차단, 버튼 클릭만
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
}

bool ALAPlayerController::IsArenaFinished() const
{
	const ALAGameState* ArenaGameState = ConnectedGameState.Get();
	if (ArenaGameState == nullptr)
	{
		return false;
	}

	return ArenaGameState->IsArenaFinished();
}
