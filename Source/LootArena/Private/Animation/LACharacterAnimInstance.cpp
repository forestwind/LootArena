#include "Animation/LACharacterAnimInstance.h"

#include "Character/LACharacterBase.h"

void ULACharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ALACharacterBase>(TryGetPawnOwner());
}

void ULACharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 에디터 미리보기 등 캐릭터 없음
	if (OwningCharacter == nullptr)
	{
		return;
	}

	Velocity = OwningCharacter->GetVelocity();
}

void ULACharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	GroundSpeed = Velocity.Size2D();
}
