#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LAMeleeTraceComponent.generated.h"

class UAnimSequenceBase;
class UStaticMeshComponent;

// 근접 판정 컴포넌트
// - 캐릭터마다 하나씩 생성 (ALACharacterBase 생성자)
// - 노티파이 스테이트는 몽타주 에셋에 하나만 있고 모든 캐릭터가 공유 → 상태는 컴포넌트에 보관
// - 노티파이 구간 동안 무기 충돌 박스를 "지난 프레임 → 이번 프레임"으로 스윕해서 맞은 적을 찾음
// - 맞은 적은 공격자에게 이벤트로 알림 (데미지 적용은 어빌리티 담당)
UCLASS()
class LOOTARENA_API ULAMeleeTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULAMeleeTraceComponent();

	// 노티파이 스테이트가 호출
	// Begin: 구간 시작 (무기 찾기, 박스 읽기, 맞은 목록 비우기)
	// Tick:  구간 동안 매 프레임 (스윕)
	// End:   구간 끝 (정리)
	// SourceAnimation: 구간이 속한 몽타주 (단계 전환 시 이전 몽타주 구간과 겹침 구분)
	void BeginMeleeTrace(const UAnimSequenceBase* SourceAnimation);
	void TickMeleeTrace(const UAnimSequenceBase* SourceAnimation);
	void EndMeleeTrace(const UAnimSequenceBase* SourceAnimation);

protected:
	// 조각 하나가 담당하는 최대 회전 각도
	// 예: 10이면 이번 프레임 무기가 45° 돌았을 때 5조각으로 나눠 스윕
	// 작을수록 정밀하지만 스윕 횟수 증가
	// ClampMin: 0이 되면 나눗셈 오류 → 최소 1
	UPROPERTY(EditDefaultsOnly, Category = "Melee Trace", meta = (ClampMin = "1.0"))
	float MaxDegreesPerSlice = 10.f;

	// 한 프레임 최대 조각 수 (렉으로 회전이 아주 클 때 스윕이 너무 많아지는 것 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Melee Trace", meta = (ClampMin = "1"))
	int32 MaxSliceCount = 8;

private:
	// 무기 메시의 Simple Collision 박스를 읽어 BoxTransformInWeapon·BoxHalfSizeInWeapon에 저장
	// 판정 모양을 바꿀 때 이 함수만 교체
	bool ReadWeaponBox();

	// 두 무기 트랜스폼 사이 회전 각도 → 조각 수
	int32 CalcSliceCount(const FTransform& FromWeaponTransform, const FTransform& ToWeaponTransform) const;

	// 두 무기 트랜스폼 사이 보간 (Ratio 0 = From, 1 = To)
	static FTransform BlendWeaponTransform(const FTransform& FromWeaponTransform, const FTransform& ToWeaponTransform, float Ratio);

	// 조각 하나 스윕: 시작 트랜스폼의 박스 → 끝 트랜스폼의 박스
	void SweepBox(const FTransform& SliceStartTransform, const FTransform& SliceEndTransform);

	// 스윕에 걸린 결과 하나 처리: 중복·대상 확인 → 이벤트 전송
	void HandleHit(const FHitResult& Hit);

	// 때려도 되는 대상인지
	bool IsValidTarget(const AActor* TargetActor) const;

	// 판정 기준 무기 컴포넌트 
	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	// 판정 중인 구간의 몽타주
	// 콤보 단계 전환 시 이전 몽타주는 블렌드 아웃 중에도 판정 구간이 남아 있음
	// → Begin(새 단계) 뒤에 End(이전 단계)가 와서 새 단계 판정이 꺼지는 문제 방지 (다른 몽타주의 Tick·End 무시)
	// → 맞음 이벤트에 담아 어느 단계 공격인지 구분
	UPROPERTY(Transient)
	TObjectPtr<const UAnimSequenceBase> TracingAnimation;

	// 이번 구간에 이미 맞은 대상
	// 스윕은 매 프레임 반복되므로 같은 적이 여러 번 걸림 → 구간당 1회만 이벤트
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> AlreadyHitActors;

	// 무기 충돌 박스 위치·회전 (무기(손) 기준 상대 좌표, 고정값)
	FTransform BoxTransformInWeapon;

	// 무기 충돌 박스 반 크기
	FVector BoxHalfSizeInWeapon = FVector::ZeroVector;

	// 지난 프레임 무기 위치·회전 (월드 좌표, 매 프레임 갱신)
	FTransform LastWeaponTransform;

	// 구간 진행 중 여부 (Begin 성공 시 true, End 시 false)
	bool bIsTracing = false;
};
