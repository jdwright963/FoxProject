// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

/**
 * Represents the current state of a save slot in the load screen UI.
 * - Vacant: The slot is empty and available for a new game
 * - EnterName: The slot is selected for a new game and awaiting player name input
 * - Taken: The slot contains an existing save with player data
 */
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant,
	EnterName,
	Taken
};

/**
 * 
 */
UCLASS()
class FOX_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	
	// The unique name identifier for this save slot, used by Unreal's save game system to identify the save file
	UPROPERTY()
	FString SlotName = FString();

	// The numeric index of this save slot (0, 1, or 2), used to identify which of the three available slots this save belongs to
	UPROPERTY()
	int32 SlotIndex = 0;

	// The player's chosen name for their character, displayed in the load screen UI with a default value if not set
	UPROPERTY()
	FString PlayerName = FString("Default Name");
	
	// The name identifier of the level/map associated with this save slot, used to determine which map to load when the player continues from this save
	UPROPERTY()
	FString MapName = FString("Default Map Name");
	
	// The tag identifier used to select which PlayerStart actor to spawn the player at when loading into a level from this save slot
	UPROPERTY()
	FName PlayerStartTag;
	
	// The current UI state of this save slot, determining whether it's empty (Vacant), awaiting name input (EnterName),
	// or contains existing save data (Taken)
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;
	
	// Indicates whether this is the player's first time loading into the game world from this save slot, used to 
	// determine if initial spawn logic should be triggered
	UPROPERTY()
	bool bFirstTimeLoadIn = true;
	
	/* Player */

	// The player's current level, representing their overall progression and unlocking new abilities and stat increases
	UPROPERTY()
	int32 PlayerLevel = 1;

	// The player's accumulated experience points, used to track progress toward leveling up
	UPROPERTY()
	int32 XP = 0;

	// The number of unspent spell points available for the player to allocate toward unlocking or upgrading abilities
	UPROPERTY()
	int32 SpellPoints = 0;

	// The number of unspent attribute points available for the player to allocate toward increasing their primary stats
	UPROPERTY()
	int32 AttributePoints = 0;

	// The player's strength attribute value, typically affecting physical damage and melee combat effectiveness
	UPROPERTY()
	float Strength = 0;

	// The player's intelligence attribute value, typically affecting spell power and magical damage effectiveness
	UPROPERTY()
	float Intelligence = 0;

	// The player's resilience attribute value, typically affecting defense and damage reduction capabilities
	UPROPERTY()
	float Resilience = 0;

	// The player's vigor attribute value, typically affecting maximum health and survivability
	UPROPERTY()
	float Vigor = 0;
};
