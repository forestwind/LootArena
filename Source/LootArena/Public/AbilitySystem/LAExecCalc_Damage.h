#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "LAExecCalc_Damage.generated.h"

// 최종 데미지 = 반올림((기본 + 공격자 Attack) × 100 / (100 + 대상 Defense)), 최소 1 → 대상 Damage 메타
UCLASS()
class LOOTARENA_API ULAExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	ULAExecCalc_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	// 공격자 Attack (Spec 생성 시점 값)
	FGameplayEffectAttributeCaptureDefinition AttackDef;

	// 대상 Defense (적용 시점 값)
	FGameplayEffectAttributeCaptureDefinition DefenseDef;
};
