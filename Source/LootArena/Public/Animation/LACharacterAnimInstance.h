#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LACharacterAnimInstance.generated.h"

class ALACharacterBase;

UCLASS()
class LOOTARENA_API ULACharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.f;

private:
	UPROPERTY(Transient)
	TObjectPtr<ALACharacterBase> OwningCharacter;

	// 게임 스레드에서 복사한 값
	FVector Velocity = FVector::ZeroVector;
};
