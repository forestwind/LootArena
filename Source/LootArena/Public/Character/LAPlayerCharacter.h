#pragma once

#include "CoreMinimal.h"
#include "Character/LACharacterBase.h"
#include "GameplayTagContainer.h"
#include "LAPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ULAInputConfig;
class ULACharacterData;
struct FInputActionValue;

UCLASS()
class LOOTARENA_API ALAPlayerCharacter : public ALACharacterBase
{
	GENERATED_BODY()

public:
	ALAPlayerCharacter();

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_AbilityPressed(FGameplayTag InputTag);
	void Input_AbilityReleased(FGameplayTag InputTag);

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<ULAInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TObjectPtr<ULACharacterData> CharacterData;
};
