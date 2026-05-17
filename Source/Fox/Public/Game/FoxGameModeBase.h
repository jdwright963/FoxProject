// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FoxGameModeBase.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class UMVVM_LoadSlot;
class USaveGame;
class UAbilityInfo;
class UCharacterClassInfo;

/**
 * 
 */
UCLASS()
class FOX_API AFoxGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	// Variable to store the UCharacterClassInfo data asset. This is set in the editor
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
	
	// Variable to store the UAbilityInfo data asset. This is set in the editor and contains configuration data for all
	// abilities in the game, including their tags, icons, materials, level requirements, and ability classes
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	// Variable to store the ULootTiers data asset. This is set in the editor and contains configuration data for
	// enemy loot drop systems, including randomized spawn chances and quantities for different item types
	UPROPERTY(EditDefaultsOnly, Category = "Loot Tiers")
	TObjectPtr<ULootTiers> LootTiers;
	
	// Saves the load slot data (player name, map name, player level, etc.) to disk at the specified slot index
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
	
	// Loads and returns save slot data from disk for the specified slot name and index, or creates a new save slot if none exists
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	
	// Deletes the save slot data from disk for the specified slot name and index (static method to allow deletion without an instance of this class)
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);
	
	// Retrieves the current in-game save data from disk using the slot name and index stored in the game instance
	ULoadScreenSaveGame* RetrieveInGameSaveData();

	// Saves the current in-game progress data to disk using the slot name and index stored in the provided save object
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);
	
	// Serializes and saves the current state of all actors in the specified world (including their properties and transforms) 
	// to the in-game save data. The optional DestinationMapAssetName parameter is used by MapEntrance actors (which inherit 
	// from Checkpoint) to specify which map should be saved as the new starting map when the player travels to a different level
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;
	
	// Loads and restores the previously saved state of the specified world (including all actors and their properties) from the in-game save data
	void LoadWorldState(UWorld* World) const;
	
	// Initiates level travel to the map associated with the provided save slot by looking up the map name in the Maps dictionary and opening the corresponding level
	void TravelToMap(UMVVM_LoadSlot* Slot);

	// The class type used to create save game instances for the load screen (stores save slot metadata like player names and slot status)
	// The value of this variable is set in the editor in a blueprint that derives from this class 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;
	
	// The name identifier of the default starting map (e.g., "OpenWorld" or "Level1") used to initialize the Maps dictionary (map)
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;

	// A soft object pointer to the default starting map's world asset, allowing lazy loading of the map when needed
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;
	
	// The tag identifier used to select the appropriate PlayerStart actor for spawning the player, allowing different spawn points based on game state (e.g., loading from a save slot)
	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	// A dictionary (map) that maps map name strings to their corresponding world asset soft pointers, enabling map lookup and travel by name
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;
	
	// Extracts and returns the simple map name from a full map asset path string (e.g., converts "/Game/Maps/Level1" to "Level1")
	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;
	
	// Overridden implementation of a blueprint native event from the parent class that selects a player spawn 
	// point by searching for a PlayerStart actor tagged with "TheTag",
	// falling back to the first available PlayerStart if no tagged one is found, or nullptr if no PlayerStarts exist in the level
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// Called when a character dies to handle death-related logic such as respawning, updating UI, or triggering game over conditions
	void PlayerDied(ACharacter* DeadCharacter);

protected:
	// Initializes the game mode by populating the Maps dictionary with the default map entry
	virtual void BeginPlay() override;
};
