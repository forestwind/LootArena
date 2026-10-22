#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LAPlayerController.generated.h"

class UInputMappingContext;
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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	// 입력 매핑 등록
	void AddDefaultMappingContext();

	// 메인 화면 위젯 생성
	void CreateHUDWidgets();

	// 인벤토리 표시 + 입력 모드·커서 전환
	void OpenInventory();
	void CloseInventory();

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

	UPROPERTY()
	TObjectPtr<ULALILootToastWidget> LootToastWidget;

	UPROPERTY()
	TObjectPtr<ULAAttributeWidget> HealthBarWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> InventoryWidget;
};
