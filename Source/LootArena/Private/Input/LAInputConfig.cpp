#include "Input/LAInputConfig.h"

#include "InputAction.h"

const UInputAction* ULAInputConfig::FindNativeInputAction(const FGameplayTag& InputTag) const
{
	const UInputAction* FoundAction = InputTagToNativeAction.FindRef(InputTag);
	if (FoundAction == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Native input action not found: %s (%s)"), *InputTag.ToString(), *GetName());
		return nullptr;
	}

	return FoundAction;
}
