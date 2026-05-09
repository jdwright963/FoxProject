// Copyright TryingToMakeGames


#include "Game/FoxGameModeBase.h"

#include "Game/FoxGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
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
	
	// Mark the save slot as "Taken" to indicate that it contains valid save data and is no longer vacant
	LoadScreenSaveGame->SaveSlotStatus = Taken;

	// Assign the map name from the load slot view model to track which level/map this save is associated with
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	
	// Store the PlayerStart tag from the load slot to determine which spawn point the player should use when loading into the map
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	
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

void AFoxGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	// Retrieve the unique slot name identifier from the load slot view model (e.g., "LoadSlot_0", "LoadSlot_1", or "LoadSlot_2")
	const FString SlotName = Slot->GetLoadSlotName();

	// Get the numeric index of the save slot (0, 1, or 2) to identify which save file to use
	const int32 SlotIndex = Slot->SlotIndex;

	// Open the level associated with this save slot by looking up the map's soft object pointer in the Maps dictionary using the map name,
	// then travel to that level (FindChecked will crash if the map name doesn't exist in the dictionary, ensuring map configuration errors are caught)
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

AActor* AFoxGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// Cast the game instance to our custom UFoxGameInstance type to access the PlayerStartTag property that determines which PlayerStart to use for spawning
	UFoxGameInstance* FoxGameInstance = Cast<UFoxGameInstance>(GetGameInstance());
	
	// Declare an array to store all PlayerStart actors found in the world
	TArray<AActor*> Actors;
	
	// Retrieve all actors of type APlayerStart from the world and populate the Actors array with them
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	
	// Check if at least one PlayerStart actor was found in the level
	if (Actors.Num() > 0)
	{
		// Initialize the selected actor with the first PlayerStart as a fallback in case no tagged PlayerStart is found
		AActor* SelectedActor = Actors[0];
		
		// Iterate through all found PlayerStart actors to search for one with the specific tag
		for (AActor* Actor : Actors)
		{
			// Attempt to cast the current actor to APlayerStart to access PlayerStart-specific properties
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// Check if this PlayerStart's tag matches the tag stored in the game instance, which determines the intended spawn location for the player
				if (PlayerStart->PlayerStartTag == FoxGameInstance->PlayerStartTag)
				{
					// Assign this tagged PlayerStart as the selected spawn point since it matches our criteria
					SelectedActor = PlayerStart;
					
					// Exit the loop early since we found the PlayerStart we were looking for
					break;
				}
			}
		}
		// Return the selected PlayerStart actor (either the one with "TheTag" or the first one found)
		return SelectedActor;
	}
	// Return nullptr if no PlayerStart actors were found in the level
	return nullptr;
}

void AFoxGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize the Maps dictionary (map) with the default map entry, associating the default map name with its world reference
	Maps.Add(DefaultMapName, DefaultMap);
}
