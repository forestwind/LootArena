#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "LAAnimNotifyState_MeleeTrace.generated.h"

class ULAMeleeTraceComponent;

// 몽타주 판정 구간 → 캐릭터의 판정 컴포넌트에 시작·매 프레임·끝 전달
// 몽타주 안에 하나만 있고 모든 캐릭터가 공유 → 상태 저장 없음
UCLASS(meta = (DisplayName = "LA Melee Trace"))
class LOOTARENA_API ULAAnimNotifyState_MeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	// 메시 주인 캐릭터의 판정 컴포넌트 (애님 에디터 미리보기에서는 nullptr)
	static ULAMeleeTraceComponent* FindMeleeTraceComponent(const USkeletalMeshComponent* MeshComp);
};
