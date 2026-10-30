#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LACharacterMovementComponent.generated.h"

// 경직 중(Character.State.HitReact) 이동·회전 정지
UCLASS()
class LOOTARENA_API ULACharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual float GetMaxSpeed() const override;
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;

private:
	bool IsHitReacting() const;
};
