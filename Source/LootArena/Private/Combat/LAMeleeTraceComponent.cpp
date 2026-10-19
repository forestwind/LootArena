#include "Combat/LAMeleeTraceComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/LACharacterBase.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Equipment/LAEquipmentComponent.h"
#include "LACollisionChannels.h"
#include "LAGameplayTags.h"
#include "PhysicsEngine/BodySetup.h"

// 디버그 드로우 켜고 끄는 콘솔 변수
// PIE 중 ~ 키 → "LA.Debug.MeleeTrace 1" 입력하면 켜짐, 0이면 꺼짐
static TAutoConsoleVariable<bool> CVarDebugMeleeTrace(
	TEXT("LA.Debug.MeleeTrace"),
	false,
	TEXT("Draw melee trace sweep boxes and hit points."));

ULAMeleeTraceComponent::ULAMeleeTraceComponent()
{
	// 판정은 공격 구간에서만 필요 → 노티파이 스테이트의 NotifyTick이 TickMeleeTrace를 대신 호출
	PrimaryComponentTick.bCanEverTick = false;
}

void ULAMeleeTraceComponent::BeginMeleeTrace(const UAnimSequenceBase* SourceAnimation)
{
	// 새 구간의 몽타주 (이전 몽타주 구간이 남아 있어도 이후 그쪽 Tick·End는 무시)
	TracingAnimation = SourceAnimation;

	// 맞은 목록 비움
	AlreadyHitActors.Reset();

	// 아래 준비가 모두 성공해야 true로 바뀜 (실패하면 Tick에서 아무것도 안 함)
	bIsTracing = false;
	
	const ULAEquipmentComponent* EquipmentComponent = GetOwner()->FindComponentByClass<ULAEquipmentComponent>();
	if (EquipmentComponent == nullptr)
	{
		return;
	}
	
	WeaponMeshComponent = EquipmentComponent->GetWeaponMeshComponent();

	// 무기가 없거나(미장착) 무기 메시에 충돌 박스가 없으면 판정 불가
	if (WeaponMeshComponent == nullptr || !ReadWeaponBox())
	{
		UE_LOG(LogTemp, Warning, TEXT("Melee trace box not found: %s"), *GetNameSafe(GetOwner()));
		return;
	}

	// 지금 무기 트랜스폼을 "지난 프레임"으로 저장 → 첫 Tick에서 여기부터 스윕
	LastWeaponTransform = WeaponMeshComponent->GetComponentTransform();
	bIsTracing = true;
}

void ULAMeleeTraceComponent::TickMeleeTrace(const UAnimSequenceBase* SourceAnimation)
{
	// 다른 몽타주(블렌드 아웃 중인 이전 단계)의 구간이면 무시
	if (SourceAnimation != TracingAnimation)
	{
		return;
	}

	// Begin 실패했거나 구간 밖이면 무시
	if (!bIsTracing || WeaponMeshComponent == nullptr)
	{
		return;
	}

	// 이번 프레임 무기 위치·회전 (월드 좌표, 무기(손) 위치)
	const FTransform CurrentWeaponTransform = WeaponMeshComponent->GetComponentTransform();

	// 무기가 많이 회전했으면 여러 조각으로 나눔 (예: 5° → 1조각, 45° → 5조각)
	const int32 SliceCount = CalcSliceCount(LastWeaponTransform, CurrentWeaponTransform);

	for (int32 SliceIndex = 0; SliceIndex < SliceCount; ++SliceIndex)
	{
		// 조각의 시작·끝 시점을 0~1 비율로 (0 = 지난 프레임, 1 = 이번 프레임)
		// 예 4조각: 0~0.25, 0.25~0.5, 0.5~0.75, 0.75~1
		// static_cast<float>: 정수 나눗셈방지
		const float SliceStartRatio = static_cast<float>(SliceIndex) / SliceCount;
		const float SliceEndRatio = static_cast<float>(SliceIndex + 1) / SliceCount;

		// 조각 시작·끝 시점의 무기 트랜스폼 (지난·이번 프레임 사이 보간)
		// 손(무기)의 위치, 무기의 방향을 구함
		const FTransform SliceStartTransform = BlendWeaponTransform(LastWeaponTransform, CurrentWeaponTransform, SliceStartRatio);
		const FTransform SliceEndTransform = BlendWeaponTransform(LastWeaponTransform, CurrentWeaponTransform, SliceEndRatio);

		// 시작 트랜스폼 → 끝 트랜스폼으로 박스 스윕
		SweepBox(SliceStartTransform, SliceEndTransform);
	}

	// 이번 프레임 트랜스폼 저장 → 다음 Tick에서 "지난 프레임"으로 사용
	LastWeaponTransform = CurrentWeaponTransform;
}

void ULAMeleeTraceComponent::EndMeleeTrace(const UAnimSequenceBase* SourceAnimation)
{
	// 다른 몽타주 구간의 끝이면 무시 (현재 진행 중인 구간 유지)
	if (SourceAnimation != TracingAnimation)
	{
		return;
	}

	// 구간 끝: Tick이 더 와도 무시하도록 끄고 정리
	bIsTracing = false;
	AlreadyHitActors.Reset();
	WeaponMeshComponent = nullptr;
	TracingAnimation = nullptr;
}

bool ULAMeleeTraceComponent::ReadWeaponBox()
{
	const UStaticMesh* WeaponMesh = WeaponMeshComponent->GetStaticMesh();

	// BodySetup: 메시의 충돌 정보 (메시 에디터에서 추가한 Simple Collision이 여기 저장됨)
	const UBodySetup* BodySetup = WeaponMesh ? WeaponMesh->GetBodySetup() : nullptr;

	// AggGeom.BoxElems: 박스 충돌 목록 (구체·캡슐은 SphereElems·SphylElems에 따로 있음)
	if (BodySetup == nullptr || BodySetup->AggGeom.BoxElems.Num() == 0)
	{
		return false;
	}

	// 무기에는 박스 하나만 두기로 함
	const FKBoxElem& Box = BodySetup->AggGeom.BoxElems[0];
	
	BoxTransformInWeapon = Box.GetTransform();

	// Box의 X/Y/Z는 박스 전체 길이
	// 스윕 모양(FCollisionShape::MakeBox)은 반 크기를 받으므로 절반으로
	BoxHalfSizeInWeapon = FVector(Box.X, Box.Y, Box.Z) * 0.5f;
	return true;
}

int32 ULAMeleeTraceComponent::CalcSliceCount(const FTransform& FromWeaponTransform, const FTransform& ToWeaponTransform) const
{
	// 무기가 몇 도 돌았나 (3D 회전이어도 "몇 도 돌았나"는 숫자 하나로 나옴)
	// AngularDistance는 라디안 → 도로 변환
	const float TurnedDegrees = FMath::RadiansToDegrees(FromWeaponTransform.GetRotation().AngularDistance(ToWeaponTransform.GetRotation()));

	// 조각 수 = 올림(회전 각도 / 조각당 최대 각도), 1 ~ MaxSliceCount 사이로 제한
	// 예 (10°, 최대 8): 5° → 1, 25° → 3, 45° → 5, 120° → 12 → 8(상한)
	return FMath::Clamp(FMath::CeilToInt(TurnedDegrees / MaxDegreesPerSlice), 1, MaxSliceCount);
}

FTransform ULAMeleeTraceComponent::BlendWeaponTransform(const FTransform& FromWeaponTransform, const FTransform& ToWeaponTransform, float Ratio)
{
	// 두 트랜스폼 사이 보간 (Ratio 0.5 = 딱 중간)
	// - 회전: Slerp = 회전용 중간값 계산
	// - 위치: Lerp = 직선 중간값 계산 (손은 조금만 움직여서 직선으로 충분)
	const FQuat BlendedRotation = FQuat::Slerp(FromWeaponTransform.GetRotation(), ToWeaponTransform.GetRotation(), Ratio);
	const FVector BlendedLocation = FMath::Lerp(FromWeaponTransform.GetLocation(), ToWeaponTransform.GetLocation(), Ratio);
	return FTransform(BlendedRotation, BlendedLocation, ToWeaponTransform.GetScale3D());
}

void ULAMeleeTraceComponent::SweepBox(const FTransform& SliceStartTransform, const FTransform& SliceEndTransform)
{
	// 받은 트랜스폼 = 손 위치 + 무기 방향 (무기 원점이 손 소켓에 붙어 있음)
	// 충돌 박스 중심 = 손에서 떨어진 거리는 BoxTransformInWeapon(손 기준 상대 좌표)
	// 박스 중심 월드 위치 = 손 위치 + (무기 방향으로 회전시킨 상대 좌표)
	// TransformPosition이 "회전 후 더하기"를 한 번에 함
	// 무기가 돌면 박스 중심도 손을 기준으로 상대좌표를 더해서 구함
	const FVector BoxStartLocation = SliceStartTransform.TransformPosition(BoxTransformInWeapon.GetLocation());
	const FVector BoxEndLocation = SliceEndTransform.TransformPosition(BoxTransformInWeapon.GetLocation());

	// 박스 방향: 스윕은 이동 중 방향 하나만 받음
	// → 이 조각의 시작·끝 회전 중간값 사용 (예: 10°→20° 조각이면 15°)
	// → 시작·끝 모두 어긋남이 절반 (조각당 10°면 ±5°, 박스 끝에서 수 cm)
	// 박스 월드 회전 = 무기 월드 회전 × 박스 상대 회전 (박스를 기울여 놓았다면 그 기울기 반영)
	const FQuat SliceMiddleRotation = FQuat::Slerp(SliceStartTransform.GetRotation(), SliceEndTransform.GetRotation(), 0.5f);
	const FQuat BoxRotation = SliceMiddleRotation * BoxTransformInWeapon.GetRotation();

	// 무기 크기 배율을 박스에도 적용 (GetAbs: 음수 배율이어도 양수로)
	const FVector BoxHalfSize = BoxHalfSizeInWeapon * SliceEndTransform.GetScale3D().GetAbs();
	const FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxHalfSize);

	// 스윕 옵션
	// - SCENE_QUERY_STAT: 프로파일러 표시 이름
	// - false: 단순 충돌(캡슐 등)로 검사 (메시 삼각형 단위 검사 안 함)
	// - GetOwner(): 자기 자신은 무시
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LAMeleeTrace), false, GetOwner());

	// 박스를 시작 → 끝 위치로 직선 이동하며(방향은 BoxRotation 고정) 걸리는 것 찾기
	// Melee 채널: Pawn 프리셋(캐릭터 캡슐)만 Overlap으로 응답, 나머지(바닥·벽)는 Ignore
	// SweepMulti: Overlap은 모두 모으고 첫 Block에서 멈춤 → Overlap이라 걸린 캐릭터 모두 반환
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, BoxStartLocation, BoxEndLocation, BoxRotation, LACollisionChannels::Melee, BoxShape, QueryParams);

	// ENABLE_DRAW_DEBUG: 배포(Shipping) 빌드에서는 이 블록이 빠짐
#if ENABLE_DRAW_DEBUG
	if (CVarDebugMeleeTrace.GetValueOnGameThread())
	{
		// 조각 끝 위치에 박스 그리기 (초록, 1초 유지)
		// 조각 박스들이 부채꼴을 덮는지 눈으로 확인
		DrawDebugBox(GetWorld(), BoxEndLocation, BoxHalfSize, BoxRotation, FColor::Green, false, 1.f);
	}
#endif

	// 걸린 결과 하나씩 처리
	for (const FHitResult& Hit : Hits)
	{
		HandleHit(Hit);
	}
}

void ULAMeleeTraceComponent::HandleHit(const FHitResult& Hit)
{
	// 걸린 컴포넌트(캡슐)의 주인 액터
	AActor* HitActor = Hit.GetActor();

	// 이번 구간에 이미 맞은 대상이면 무시 (매 프레임·조각마다 같은 적이 다시 걸림)
	if (HitActor == nullptr || AlreadyHitActors.Contains(HitActor))
	{
		return;
	}

	// 때려도 되는 대상인지 확인 (아군·사망·준비 안 됨 제외)
	if (!IsValidTarget(HitActor))
	{
		return;
	}

	// 맞은 목록에 추가 → 이번 구간에서는 다시 처리 안 함
	AlreadyHitActors.Add(HitActor);

#if ENABLE_DRAW_DEBUG
	if (CVarDebugMeleeTrace.GetValueOnGameThread())
	{
		// 맞은 지점에 점 그리기 (빨강, 크기 12, 1초 유지)
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 12.f, FColor::Red, false, 1.f);
	}
#endif

	// 공격자에게 이벤트 보내기
	FGameplayEventData Payload;
	Payload.EventTag = LAGameplayTags::Event_Melee_Hit;
	// 때린 쪽
	Payload.Instigator = GetOwner();
	// 맞은 쪽
	Payload.Target = HitActor;
	// 맞은 지점·방향 등 스윕 결과 전체를 GAS 형식(TargetData)으로 감싸서 담음
	// 나중에 피격 이펙트 위치·히트 리액트 방향에 사용
	Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
	// 판정 구간의 몽타주 (공격 어빌리티가 어느 단계 공격인지 구분)
	Payload.OptionalObject = TracingAnimation;

	// 공격자 자신의 ASC로 이벤트 전송
	// 공격자의 실행 중인 어빌리티 중 WaitGameplayEvent 기다리는 태스크가 받음
	// 판정 컴포넌트는 "맞았다"만 알리고 데미지 규칙은 어빌리티가 결정
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), LAGameplayTags::Event_Melee_Hit, Payload);
}

bool ULAMeleeTraceComponent::IsValidTarget(const AActor* TargetActor) const
{
	// LootArena 캐릭터만 (다른 종류 액터가 Melee 채널에 응답해도 무시)
	// 자기 자신 제외 (스윕에서도 무시하지만 한 번 더 확인)
	const ALACharacterBase* TargetCharacter = Cast<ALACharacterBase>(TargetActor);
	if (TargetCharacter == nullptr || TargetCharacter == GetOwner())
	{
		return false;
	}

	// 캐릭터 데이터 적용 전이면 제외(로드 전 등)
	// 적은 데이터를 비동기 로딩 → 로딩 전에는 스탯·어빌리티가 없어 데미지 처리 불가
	if (!TargetCharacter->IsCharacterDataApplied())
	{
		return false;
	}

	// 적대 관계 체크
	// GetAttitude(나, 상대): 두 캐릭터의 TeamId 비교 (엔진 기본: 같으면 아군, 다르면 적대)
	if (FGenericTeamId::GetAttitude(GetOwner(), TargetActor) != ETeamAttitude::Hostile)
	{
		return false;
	}

	// 이미 죽은 대상 제외
	const UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();
	if (TargetASC != nullptr && TargetASC->HasMatchingGameplayTag(LAGameplayTags::Character_State_Dead))
	{
		return false;
	}

	return true;
}
