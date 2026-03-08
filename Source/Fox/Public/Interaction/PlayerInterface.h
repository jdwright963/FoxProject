// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOX_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// Function to find the level for a given XP amount this is overriden in FoxCharacter.h 
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP) const;

	// Function to get the characters current XP this is overriden in FoxCharacter.h 
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	// Function to get the characters attribute points reward for a given level this is overriden in FoxCharacter.h 
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level) const;

	// Function to get the characters spell points reward for a given level this is overriden in FoxCharacter.h 
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level) const;
	
	/**
	 * AddToXP - Adds experience points to the player character
	 * 
	 * WHY THIS INTERFACE EXISTS:
	 * - AFoxPlayerState owns UFoxAttributeSet (PlayerState needs to include AttributeSet.h)
	 * - UFoxAttributeSet needs to talk to AFoxPlayerState when XP changes (AttributeSet would need to include PlayerState.h)
	 * - This creates a circular include/dependency: PlayerState includes AttributeSet, AttributeSet includes
	 *   PlayerState, and this repeats
	 * - Circular dependencies will cause compilation errors because the compiler cannot determine which header to process
	 *   first, resulting in incomplete type definitions and "undefined type" errors when trying to use class members.
	 * 
	 * The Fix:
	 * 1. Enemy dies, gives XP
	 * 2. UFoxAttributeSet detects XP change
	 * 3. AttributeSet calls AddToXP() on the character (as IPlayerInterface)
	 * 4. AFoxCharacter::AddToXP_Implementation() overrides this function
	 * 5. inside this function AFoxCharacter gets its PlayerState and calls PlayerState->AddToXP() which is its own 
	 *    AddToXP() function
	 * 6. AFoxPlayerState does the actual work (add XP, check for level up, etc.)
	 * 
	 * AFoxCharacter is the middleman - it overrides this function but just passes the work to PlayerState.
	 * No circular include: PlayerState -> AttributeSet -> Interface
	 * 
	 * BLUEPRINTNATIVEEVENT:
	 * 
	 * This is a BlueprintNativeEvent:
	 * - You override it in C++ by writing AddToXP_Implementation()
	 * - You can also override it in Blueprint
	 * - Blueprint overrides replace the C++ version
	 * 
	 * @param InXP How much XP to add
	*/
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);
	
	// Function to increase the player's level by the specified amount this is overriden in FoxCharacter.h
	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	// Function to add the specified amount of attribute points to the character this is overriden in FoxCharacter.h
	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);
	
	// Function to get the character's current attribute points this is overriden in FoxCharacter.h
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePoints() const;

	// Function to add the specified amount of spell points to the character this is overriden in FoxCharacter.h
	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(int32 InSpellPoints);
	
	// Function to get the character's current spell points this is overriden in FoxCharacter.h
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPoints() const;
	
	// Function to handle character leveling up this is overriden in FoxCharacter.h 
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();
};
