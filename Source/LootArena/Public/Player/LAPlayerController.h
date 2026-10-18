#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LAPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class LOOTARENA_API ALAPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
