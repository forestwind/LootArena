#include "UI/LAAttributeWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/ProgressBar.h"

void ULAAttributeWidget::SetTargetActor(AActor* InTargetActor)
{
	ClearTarget();

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InTargetActor);
	if (!ASC)
	{
		RefreshValues();
		return;
	}

	// 속성 소속 세트가 ASC 세트 목록에 있는지 (주인 액터 서브오브젝트 세트는 ASC InitializeComponent에서 자동 등록)
	if (!ASC->HasAttributeSetForAttribute(ValueAttribute) || !ASC->HasAttributeSetForAttribute(MaxValueAttribute))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttributeWidget: attribute not found on %s (%s)"), *GetNameSafe(InTargetActor), *GetName());
		RefreshValues();
		return;
	}

	TargetASC = ASC;
	ValueChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(ValueAttribute).AddUObject(this, &ThisClass::HandleAttributeChanged);
	MaxValueChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(MaxValueAttribute).AddUObject(this, &ThisClass::HandleAttributeChanged);

	// 연결 시 현재 값 (변경 알림은 처음 값을 보내지 않음)
	RefreshValues();
}

void ULAAttributeWidget::NativeDestruct()
{
	ClearTarget();

	Super::NativeDestruct();
}

void ULAAttributeWidget::ClearTarget()
{
	if (UAbilitySystemComponent* ASC = TargetASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(ValueAttribute).Remove(ValueChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(MaxValueAttribute).Remove(MaxValueChangedHandle);
	}

	ValueChangedHandle.Reset();
	MaxValueChangedHandle.Reset();
	TargetASC.Reset();
}

void ULAAttributeWidget::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	RefreshValues();
}

void ULAAttributeWidget::RefreshValues()
{
	float Value = 0.f;
	float MaxValue = 0.f;

	if (UAbilitySystemComponent* ASC = TargetASC.Get())
	{
		Value = ASC->GetNumericAttribute(ValueAttribute);
		MaxValue = ASC->GetNumericAttribute(MaxValueAttribute);
	}

	// 최대 값 0 이하 → 비율 0
	float Ratio = 0.f;
	if (MaxValue > 0.f)
	{
		Ratio = FMath::Clamp(Value / MaxValue, 0.f, 1.f);
	}

	if (ValueBar)
	{
		ValueBar->SetPercent(Ratio);
	}

	OnValuesUpdated(Value, MaxValue);
}
