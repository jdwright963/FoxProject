// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;

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
 * Represents the complete saved state of a single actor instance for persistence across game sessions.
 * This struct stores three critical pieces of information: the actor's unique identifier (ActorName),
 * its spatial transformation in the world (Transform), and serialized property data (Bytes).
 * The Bytes array contains only properties marked with the SaveGame specifier, which are automatically
 * serialized from the actor and can be restored when loading. Used by FSavedMap to store multiple
 * actor states within a single level/map. The equality operator (==) is overloaded to compare actors
 * based solely on ActorName, allowing efficient searching and duplicate detection in saved actor arrays.
 */
USTRUCT()
struct FSavedActor
{
	GENERATED_BODY()

	// The unique name identifier for this saved actor, used to match and restore the correct actor instance when loading the saved game
	UPROPERTY()
	FName ActorName = FName();

	// The complete spatial transformation of the actor including position, rotation, and scale in 3D world space at the time of save
	UPROPERTY()
	FTransform Transform = FTransform();

	// Serialized variables from the Actor - only those marked with SaveGame specifier
	// Raw binary data containing all SaveGame-marked properties from the actor, serialized into a byte array for persistent storage and restoration
	UPROPERTY()
	TArray<uint8> Bytes;
};

// Overloading the equality operator (==) to compare two FSavedActor instances for equality based solely on their ActorName
// Two actors are considered equal if their ActorName values match exactly, ignoring Transform and Bytes fields
// Returns true if the ActorName values match exactly, false otherwise
// Used for searching and detecting duplicate actors in saved actor arrays
inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
	return Left.ActorName == Right.ActorName;
}

/**
 * Represents the complete saved state of a single map/level including all persistent actors within it.
 * This struct stores the map's asset identifier and an array of all serialized actors that existed in the map
 * at save time. Used by ULoadScreenSaveGame to persist entire level states across game sessions, allowing
 * the game world to be restored exactly as it was when the player saved, including actor positions, properties,
 * and states. Each FSavedMap entry corresponds to one visited level in the game.
 */
USTRUCT()
struct FSavedMap
{
	GENERATED_BODY()

	// The asset path identifier of this saved map (e.g., "/Game/Maps/Level1"), used to identify which level this saved 
	// state belongs to and to load the correct map when restoring
	UPROPERTY()
	FString MapAssetName = FString();

	// Array containing all serialized actor instances that existed in this map at the time of save, each storing the 
	// actor's name and transform data for complete state restoration
	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};

/**
 * Represents a saved gameplay ability with all its persistent data for serialization and restoration.
 * This struct stores the complete state of a player's ability including its class type, unique identifier,
 * current status (locked/unlocked), UI slot assignment, ability classification, and upgrade level.
 * Used by ULoadScreenSaveGame to persist ability data across game sessions, allowing the player's
 * ability configuration to be saved and restored when loading a game.
 */
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()
	
	// The class reference of the gameplay ability, defining the actual ability behavior and implementation to be instantiated
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	// The unique gameplay tag identifier for this ability, used to reference and distinguish this specific ability from others in the system
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();

	// The gameplay tag representing the current status of the ability (e.g., locked, unlocked, equipped), determining availability and usage
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag();

	// The gameplay tag identifying which UI slot or input binding this ability is assigned to for player access and activation
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot = FGameplayTag();

	// The gameplay tag categorizing the type of ability (e.g., offensive, defensive, passive), used for filtering and organization
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag();

	// The current upgrade level of the ability, determining its power and effectiveness with higher levels providing stronger effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 1;
};


// Overloading the equality operator (==) to compare two FSavedAbility instances for equality based solely on their AbilityTag
// Two abilities are considered equal if their AbilityTag values match exactly, ignoring all other fields
// Returns true if the AbilityTag values match exactly, false otherwise
inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

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

	// The asset path identifier of the map associated with this save slot (e.g., "/Game/Maps/Level1"), used to uniquely identify
	// and load the correct map asset when restoring the game from this save
	UPROPERTY()
	FString MapAssetName = FString("Default Map Asset Name");
	
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
	
	/* Abilities */

	// Array of all abilities the player has acquired, storing their configuration including ability class, tags for identification,
	// status (locked/unlocked), assigned slot, type classification, and current level for each ability
	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;
	
	// Array of all maps/levels that have been visited and saved, storing the map asset name and all serialized actors within each map
	// for complete level state persistence and restoration across game sessions
	UPROPERTY()
	TArray<FSavedMap> SavedMaps;

	// Retrieves the saved map struct for a specific map by its asset name, used to restore the complete state of actors
	// and their properties when loading into that level
	FSavedMap GetSavedMapWithMapName(const FString& InMapName);

	// Checks whether a map with the specified asset name exists in the SavedMaps array, used to determine if a level
	// has previously been visited and saved in this saved game
	bool HasMap(const FString& InMapName);
};
