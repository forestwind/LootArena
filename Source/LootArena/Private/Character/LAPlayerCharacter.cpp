#include "Character/LAPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Character/LACharacterData.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/LAInputConfig.h"
#include "InputActionValue.h"
#include "LAGameplayTags.h"

ALAPlayerCharacter::ALAPlayerCharacter()
{
	// 컨트롤러 회전은 카메라만 사용
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 이동 방향으로 회전
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ALAPlayerCharacter::PossessedBy(AController* NewController)
{
	// 베이스에서 InitAbilityActorInfo 먼저
	Super::PossessedBy(NewController);

	ApplyCharacterData(CharacterData);
}

void ALAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Enhanced Input Component required: %s"), *GetName());
		return;
	}

	if (InputConfig == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("InputConfig not set: %s"), *GetName());
		return;
	}

	const UInputAction* MoveAction = InputConfig->FindNativeInputAction(LAGameplayTags::InputTag_Move);
	if (MoveAction != nullptr)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	}

	const UInputAction* LookAction = InputConfig->FindNativeInputAction(LAGameplayTags::InputTag_Look);
	if (LookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	}
}

void ALAPlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();

	// 카메라 수평 방향 기준
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MoveValue.Y);
	AddMovementInput(RightDirection, MoveValue.X);
}

void ALAPlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}
