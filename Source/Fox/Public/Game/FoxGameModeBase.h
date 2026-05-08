// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FoxGameModeBase.generated.h"

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
	
	// Saves the load slot data (player name, map name, player level, etc.) to disk at the specified slot index
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
	
	// Loads and returns save slot data from disk for the specified slot name and index, or creates a new save slot if none exists
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	
	// Deletes the save slot data from disk for the specified slot name and index (static method to allow deletion without an instance of this class)
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);

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

	// A dictionary (map) that maps map name strings to their corresponding world asset soft pointers, enabling map lookup and travel by name
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

protected:
	// Initializes the game mode by populating the Maps dictionary with the default map entry
	virtual void BeginPlay() override;
};
