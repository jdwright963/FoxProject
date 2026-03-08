// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOX_API IEnemyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// = 0 makes this a pure virtual function. Which means these functions must be overridden make a child of this class.
	virtual void HighlightActor() = 0;
	virtual void UnHighlightActor() = 0;
	
	// Blueprint Native Events for an interface allows us to create an event that can be implemented in and called from 
	// blueprints. These should not be marked virtual and they auto generate an implementation version 
	// (SetCombatTarget_Implementation) that can be overridden. This is a function that sets the combat target for the
	// enemy
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);

	// This is a function that gets the combat target for the enemy
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;
};
