// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
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

	// Function to apply visual highlighting to the actor that implements this interface when it's targeted or hovered over
	UFUNCTION(BlueprintNativeEvent)
	void HighlightActor();

	// Function to remove visual highlighting from the actor that implements this interface when it's no longer targeted or hovered over
	UFUNCTION(BlueprintNativeEvent)
	void UnHighlightActor();
	
	// Function to set the destination location for player movement when the actor implementing this interface is clicked, 
	// allowing the implementer to specify where the player should navigate to
	UFUNCTION(BlueprintNativeEvent)
	void SetMoveToLocation(FVector& OutDestination);
};
