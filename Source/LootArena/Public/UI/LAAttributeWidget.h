#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "LAAttributeWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;

// 현재 값 / 최대 값 속성 한 쌍 막대 표시 (레이아웃·연출은 자식 BP)
UCLASS(Abstract)
class LOOTARENA_API ULAAttributeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 표시 대상 지정 (nullptr이면 해제)
	void SetTargetActor(AActor* InTargetActor);

protected:
	virtual void NativeDestruct() override;

	// 현재 값 속성 (예: Health)
	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	FGameplayAttribute ValueAttribute;

	// 최대 값 속성 (예: MaxHealth)
	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	FGameplayAttribute MaxValueAttribute;

	// 비율 막대 (자식 BP에 같은 이름 ProgressBar 필수)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ValueBar;

	// 값 갱신 알림 (숫자 텍스트·연출용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Attribute")
	void OnValuesUpdated(float Value, float MaxValue);

private:
	void ClearTarget();
	void HandleAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void RefreshValues();

	TWeakObjectPtr<UAbilitySystemComponent> TargetASC;

	FDelegateHandle ValueChangedHandle;
	FDelegateHandle MaxValueChangedHandle;
};
