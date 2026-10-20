#include "AbilitySystem/LAExecCalc_Damage.h"

#include "AbilitySystem/LACharacterAttributeSet.h"
#include "LAGameplayTags.h"

ULAExecCalc_Damage::ULAExecCalc_Damage()
{
	// 스냅샷 true: Spec 생성 시점 값 고정 / false: 적용 시점 값 사용
	AttackDef =FGameplayEffectAttributeCaptureDefinition(ULACharacterAttributeSet::GetAttackAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	DefenseDef = FGameplayEffectAttributeCaptureDefinition(ULACharacterAttributeSet::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);

	RelevantAttributesToCapture.Add(AttackDef);
	RelevantAttributesToCapture.Add(DefenseDef);
}

void ULAExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 단계별 기본 데미지 (없으면 경고 로그)
	const float BaseDamage = Spec.GetSetByCallerMagnitude(LAGameplayTags::Data_Damage_Base, true, 0.f);

	float Attack = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AttackDef, EvaluateParams, Attack);
	Attack = FMath::Max(Attack, 0.f);

	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DefenseDef, EvaluateParams, Defense);
	Defense = FMath::Max(Defense, 0.f);

	// 정수 데미지 (반올림 후 최소 1)
	const float RawDamage = (BaseDamage + Attack) * 100.f / (100.f + Defense);
	const float FinalDamage = FMath::Max(FMath::RoundToFloat(RawDamage), 1.f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULACharacterAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}
