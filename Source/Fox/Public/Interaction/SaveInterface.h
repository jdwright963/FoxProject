// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOX_API ISaveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Determines whether the actor's transform (location, rotation, scale) should be loaded from saved data
	// Returns true if the transform should be restored, false if the actor should keep its default placed transform
	// BlueprintNativeEvent allows both C++ and Blueprint implementations with C++ providing default behavior
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ShouldLoadTransform();

	// Called to restore the actor's saved state and properties from the save game data
	// Implementing classes should override this to load their specific saved data and restore the actor to its saved state
	// BlueprintNativeEvent allows both C++ and Blueprint implementations with C++ providing default behavior
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadActor();
};
