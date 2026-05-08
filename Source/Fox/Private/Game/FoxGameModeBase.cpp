// Copyright TryingToMakeGames


#include "Game/FoxGameModeBase.h"

#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void AFoxGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// Check if a save game already exists in this slot with the specified name
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		// Delete the existing save game to prevent conflicts and ensure a clean save
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	
	// Create a new save game object instance using the USaveGame type
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	
	// Cast the generic save game object to our specific ULoadScreenSaveGame type to access custom properties
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	
	// Assign the player name from the load slot view model to the save game object
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	
	LoadScreenSaveGame->SaveSlotStatus = Taken;
	
	/*
	 * Write the save game object to persistent storage
	 * Parameters:
	 * - LoadScreenSaveGame: The save game object containing the data to be saved (player name, etc.)
	 * - LoadSlot->GetLoadSlotName(): The unique string identifier for this save slot (e.g., "LoadSlot_0")
	 * - SlotIndex: The numeric index of the save slot (0, 1, or 2) used for organizing multiple save files
	 */
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

ULoadScreenSaveGame* AFoxGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	// Initialize a pointer to hold the save game object, starting as nullptr until we determine if we load or create one
	USaveGame* SaveGameObject = nullptr;
	
	// Check if a save game file already exists at the specified slot name and index
	// DoesSaveGameExist is an engine-defined function from UGameplayStatics that checks if a save file exists on disk
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// Load the existing save game data from disk into the SaveGameObject
		// LoadGameFromSlot is an engine-defined function from UGameplayStatics that deserializes save data from disk
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		// No save exists, so create a new save game object with default values using our LoadScreenSaveGameClass
		// CreateSaveGameObject is an engine-defined function from UGameplayStatics that instantiates a new save game object
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	// Cast the generic USaveGame pointer to our specific ULoadScreenSaveGame type to access custom properties
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	
	// Return the save game object, either loaded from disk if it existed or newly created with default values
	return LoadScreenSaveGame;
}

void AFoxGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// Check if a save game file exists at the specified slot name and index before attempting to delete it
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// Delete the save game file from disk, removing all saved data associated with this slot
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

void AFoxGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the Maps dictionary (map) with the default map entry, associating the default map name with its world reference
	Maps.Add(DefaultMapName, DefaultMap);
}
