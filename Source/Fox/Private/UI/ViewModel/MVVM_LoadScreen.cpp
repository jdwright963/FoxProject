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
	
}
