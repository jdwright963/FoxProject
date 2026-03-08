// Copyright TryingToMakeGames


#include "Input/FoxInputConfig.h"

const UInputAction* UFoxInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	// Loops through the items in the AbilityInputActions array
	for (const FFoxInputAction& Action : AbilityInputActions)
	{
		// Checks if the current element in the array has an input action that is not null and the input tag of the 
		// current element matches the InputTag input parameter of this function. If so, the input action is returned.
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s], on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}
	
	// If the if statement in the for loop fails every iteration
	return nullptr;
}
