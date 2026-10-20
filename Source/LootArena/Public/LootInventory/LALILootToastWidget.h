#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LIStructTypes.h"
#include "LALILootToastWidget.generated.h"

// 획득 토스트 순차 표시 (모양·연출은 자식 BP)
UCLASS(Abstract)
class LOOTARENA_API ULALILootToastWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 토스트 하나 표시 시간 (초)
	UPROPERTY(EditDefaultsOnly, Category = "Toast")
	float ShowDuration = 2.f;

	// 토스트 사이 간격 (초, 숨김 연출 시간)
	UPROPERTY(EditDefaultsOnly, Category = "Toast")
	float HideDuration = 0.3f;

	// 토스트 표시 시작
	UFUNCTION(BlueprintImplementableEvent, Category = "Toast")
	void OnShowToast(const FLIItemDisplayInfo& Info, int32 Count);

	// 토스트 표시 끝
	UFUNCTION(BlueprintImplementableEvent, Category = "Toast")
	void OnHideToast();

private:
	UFUNCTION()
	void HandleItemAdded(int32 ItemId, int32 Count);

	void ShowNext();
	void FinishCurrent();

	// 대기 중인 토스트 (ItemId + Count)
	TArray<FLIInventoryEntry> PendingToasts;

	// 표시 중 또는 간격 대기 중
	bool bShowing = false;

	FTimerHandle ToastTimerHandle;
};
