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
};
