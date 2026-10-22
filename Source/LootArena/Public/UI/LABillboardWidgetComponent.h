#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "LABillboardWidgetComponent.generated.h"

// World 공간 위젯 화면 평행 회전
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class LOOTARENA_API ULABillboardWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
