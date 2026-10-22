#include "UI/LABillboardWidgetComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

void ULABillboardWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsVisible())
	{
		return;
	}

	// 화면과 평행 (카메라 정면 반대 방향)
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager)
	{
		const FVector CameraForward = CameraManager->GetCameraRotation().Vector();
		SetWorldRotation((-CameraForward).Rotation());
	}
}
