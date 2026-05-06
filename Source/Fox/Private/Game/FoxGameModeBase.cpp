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
	
	/*
	 * Write the save game object to persistent storage
	 * Parameters:
	 * - LoadScreenSaveGame: The save game object containing the data to be saved (player name, etc.)
	 * - LoadSlot->GetLoadSlotName(): The unique string identifier for this save slot (e.g., "LoadSlot_0")
	 * - SlotIndex: The numeric index of the save slot (0, 1, or 2) used for organizing multiple save files
	 */
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}
