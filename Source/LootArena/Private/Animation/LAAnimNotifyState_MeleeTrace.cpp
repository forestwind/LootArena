#include "Animation/LAAnimNotifyState_MeleeTrace.h"

#include "Combat/LAMeleeTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"

void ULAAnimNotifyState_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ULAMeleeTraceComponent* MeleeTraceComponent = FindMeleeTraceComponent(MeshComp);
	if (MeleeTraceComponent != nullptr)
	{
		MeleeTraceComponent->BeginMeleeTrace(Animation);
	}
}

void ULAAnimNotifyState_MeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	ULAMeleeTraceComponent* MeleeTraceComponent = FindMeleeTraceComponent(MeshComp);
	if (MeleeTraceComponent != nullptr)
	{
		MeleeTraceComponent->TickMeleeTrace(Animation);
	}
}

void ULAAnimNotifyState_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ULAMeleeTraceComponent* MeleeTraceComponent = FindMeleeTraceComponent(MeshComp);
	if (MeleeTraceComponent != nullptr)
	{
		MeleeTraceComponent->EndMeleeTrace(Animation);
	}
}

ULAMeleeTraceComponent* ULAAnimNotifyState_MeleeTrace::FindMeleeTraceComponent(const USkeletalMeshComponent* MeshComp)
{
	if (MeshComp == nullptr || MeshComp->GetOwner() == nullptr)
	{
		return nullptr;
	}

	return MeshComp->GetOwner()->FindComponentByClass<ULAMeleeTraceComponent>();
}
