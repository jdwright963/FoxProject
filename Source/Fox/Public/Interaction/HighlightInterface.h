// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UHighlightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOX_API IHighlightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Pure virtual function (= 0) means this function has no implementation here and MUST be overridden by any class 
	// that implements this interface. This ensures all implementing classes provide their own highlighting behavior.
	// This is a function that applies visual highlighting effect to the actor, typically used for cursor hover or selection feedback
	virtual void HighlightActor() = 0;

	// Pure virtual function (= 0) means this function has no implementation here and MUST be overridden by any class 
	// that implements this interface. This ensures all implementing classes provide their own unhighlighting behavior.
	// This is a function that removes visual highlighting effect from the actor when it's no longer targeted or selected
	virtual void UnHighlightActor() = 0;
};
