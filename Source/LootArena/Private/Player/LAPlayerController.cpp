#include "Player/LAPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ALAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어만 입력 사용
	if (!IsLocalController())
	{
		return;
	}

	if (DefaultMappingContext == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext not set: %s"), *GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSubsystem == nullptr)
	{
		return;
	}

	InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
}
