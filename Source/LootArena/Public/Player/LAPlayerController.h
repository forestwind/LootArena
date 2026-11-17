#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LAPlayerController.generated.h"

class ALAGameState;
class UInputMappingContext;
class ULAArenaStateWidget;
class ULAAttributeWidget;
class ULAInputConfig;
class ULALILootToastWidget;
class UUserWidget;

UCLASS()
class LOOTARENA_API ALAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 인벤토리 열기/닫기 전환
	void ToggleInventory();

	// 슬롯 사용 요청 (UI 클릭 → 폰에 Event.Item.Use)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseInventorySlot(int32 SlotId);

	// 결과 화면 버튼: 현재 레벨 다시 열기
	UFUNCTION(BlueprintCallable, Category = "Arena")
	void RestartArena();

	// 결과 화면 버튼: 게임 종료
	UFUNCTION(BlueprintCallable, Category = "Arena")
	void QuitArena();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	// 입력 매핑 등록
	void AddDefaultMappingContext();

	// 메인 화면 위젯 생성
	void CreateHUDWidgets();

	// 클래스 확인 → 생성 → 화면 추가 (클래스 없으면 경고 후 nullptr)
	UUserWidget* CreateViewportWidget(TSubclassOf<UUserWidget> WidgetClass, const TCHAR* ClassPropertyName);

	// 인벤토리 표시 + 입력 모드·커서 전환
	void OpenInventory();
	void CloseInventory();

	// 게임 전용 입력 + 커서 숨김
	void SetGameInputMode();

	// GameState 알림 구독 / 해제
	void ConnectGameState();
	void DisconnectGameState();

	// 승리·패배 시 인벤토리 닫기 → 대기 후 결과 화면
	void HandleArenaStateChanged();

	// 결과 위젯 표시 + UI 전용 입력
	void ShowArenaResult();

	bool IsArenaFinished() const;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// UI 입력 (폰과 무관)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<ULAInputConfig> UIInputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULALILootToastWidget> LootToastWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULAAttributeWidget> HealthBarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULAArenaStateWidget> ArenaStageWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULAArenaStateWidget> ArenaResultWidgetClass;

	// 승리·패배 → 결과 화면 대기 (초)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float ResultDelay = 2.f;

	UPROPERTY()
	TObjectPtr<ULALILootToastWidget> LootToastWidget;

	UPROPERTY()
	TObjectPtr<ULAAttributeWidget> HealthBarWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<ULAArenaStateWidget> ArenaStageWidget;

	UPROPERTY()
	TObjectPtr<ULAArenaStateWidget> ArenaResultWidget;

	FTimerHandle ResultTimerHandle;

	TWeakObjectPtr<ALAGameState> ConnectedGameState;

	FDelegateHandle ArenaStateChangedHandle;
};
