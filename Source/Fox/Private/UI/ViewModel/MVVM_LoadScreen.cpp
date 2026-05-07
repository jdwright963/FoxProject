// Copyright TryingToMakeGames


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/FoxGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	// Create a new view model instance of type LoadSlotViewModelClass for the first load slot
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Assign the unique identifier name to the first load slot
	LoadSlot_0->SetLoadSlotName(FString("LoadSlot_0"));
	
	// Register the first load slot in the map with index 0 for quick lookup
	LoadSlots.Add(0, LoadSlot_0);
	
	// Create a new view model instance of type LoadSlotViewModelClass for the second load slot
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Register the second load slot in the map with index 1 for quick lookup
	LoadSlots.Add(1, LoadSlot_1);
	
	// Assign the unique identifier name to the second load slot
	LoadSlot_1->SetLoadSlotName(FString("LoadSlot_1"));
	
	// Create a new view model instance of type LoadSlotViewModelClass for the third load slot
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Register the third load slot in the map with index 2 for quick lookup
	LoadSlots.Add(2, LoadSlot_2);
	
	// Assign the unique identifier name to the third load slot
	LoadSlot_2->SetLoadSlotName(FString("LoadSlot_2"));
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{ 
	// Retrieve and return the load slot view model from the map using the given index (asserts if index not found)
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	// Retrieve the current game mode and cast it to FoxGameModeBase to access save functionality
	AFoxGameModeBase* FoxGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this));

	// Assign the player-entered name to the selected load slot's player name field
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	
	// Mark the slot status as Taken to indicate this slot now contains valid save data
	LoadSlots[Slot]->SlotStatus = Taken;

	// Save the load slot data to persistent storage via the game mode's save system
	FoxGameMode->SaveSlotData(LoadSlots[Slot], Slot);

	// Initialize the slot to refresh its state and update its UI representation
	LoadSlots[Slot]->InitializeSlot();
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	// Broadcast widget switcher index to show the name entry interface for creating a new game in this slot
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	// Notify all bound listeners (e.g., UI widgets) that a save slot has been selected, triggering any registered callback functions.
	SlotSelected.Broadcast();
	
	// Iterate through all load slots in the LoadSlots map to update their select button states based on the currently selected slot
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// Check if the current load slot's index matches the slot that was just selected by the player
		if (LoadSlot.Key == Slot)
		{
			// Broadcast false to disable the select button for this slot since it's now the active selection
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);
		}
		// If this is NOT the currently selected slot, we need to enable its select button so it can be clicked
		else
		{
			// Broadcast true to enable the select button for this unselected slot, allowing the player to switch to it
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);
		}
	}
}

void UMVVM_LoadScreen::LoadData()
{
	// Retrieve the current game mode and cast it to FoxGameModeBase to access save slot data functionality
	AFoxGameModeBase* FoxGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this));

	// Iterate through all registered load slots in the map to load their saved data
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		/*
		 * Call GetSaveSlotData to retrieve the ULoadScreenSaveGame object from persistent storage for this slot
		 * GetSaveSlotData is a function on AFoxGameModeBase that loads save data from disk
		 * First parameter: LoadSlot.Value->GetLoadSlotName() - LoadSlot is a TTuple<int32, UMVVM_LoadSlot*> where
		 *   .Value accesses the UMVVM_LoadSlot* pointer (the second element of the tuple), then we call
		 *   GetLoadSlotName() on that view model to retrieve the FString slot name (e.g., "LoadSlot_0")
		 * Second parameter: LoadSlot.Key - accesses the int32 index (the first element of the tuple) representing
		 *   the slot number (0, 1, or 2) used as an additional identifier for the save slot
		 */
		ULoadScreenSaveGame* SaveObject = FoxGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		// Extract the player's name from the save object
		const FString PlayerName = SaveObject->PlayerName;

		// Extract the slot status (e.g., Empty, Taken) from the save object
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;

		// Assign the loaded slot status to the view model's SlotStatus property 
		// .Value accesses the UMVVM_LoadSlot* pointer (the second element of the tuple)
		LoadSlot.Value->SlotStatus = SaveSlotStatus;

		// Assign the loaded player name to the as the player name for the current load slot
		LoadSlot.Value->SetPlayerName(PlayerName);

		// Initialize the slot to refresh its state and update its UI representation with the loaded data
		LoadSlot.Value->InitializeSlot();
	}
}
