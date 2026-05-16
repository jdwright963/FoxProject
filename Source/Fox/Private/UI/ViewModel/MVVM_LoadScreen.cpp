// Copyright TryingToMakeGames


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/FoxGameInstance.h"
#include "Game/FoxGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	// Create a new view model instance of type LoadSlotViewModelClass for the first load slot
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Assign the unique identifier name to the first load slot
	LoadSlot_0->SetLoadSlotName(FString("LoadSlot_0"));
	
	// Set the slot index to 0 to identify this as the first save slot
	LoadSlot_0->SlotIndex = 0;
	
	// Register the first load slot in the map with index 0 for quick lookup
	LoadSlots.Add(0, LoadSlot_0);
	
	// Create a new view model instance of type LoadSlotViewModelClass for the second load slot
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Register the second load slot in the map with index 1 for quick lookup
	LoadSlots.Add(1, LoadSlot_1);
	
	// Assign the unique identifier name to the second load slot
	LoadSlot_1->SetLoadSlotName(FString("LoadSlot_1"));
	
	// Set the slot index to 1 to identify this as the second save slot
	LoadSlot_1->SlotIndex = 1;
	
	// Create a new view model instance of type LoadSlotViewModelClass for the third load slot
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	
	// Register the third load slot in the map with index 2 for quick lookup
	LoadSlots.Add(2, LoadSlot_2);
	
	// Assign the unique identifier name to the third load slot
	LoadSlot_2->SetLoadSlotName(FString("LoadSlot_2"));
	
	// Set the slot index to 2 to identify this as the third save slot
	LoadSlot_2->SlotIndex = 2;
	
	// Update the NumLoadSlots property with the total number of registered load slots, broadcasting the count to bound UI elements
	SetNumLoadSlots(LoadSlots.Num());
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
	
	// Validate that the game mode cast succeeded. This will fail if not running in single player or if the game mode is not FoxGameModeBase
	if (!IsValid(FoxGameMode))
	{
		// Display an on-screen debug message to inform the player/developer they must switch to single player mode for save functionality to work
		GEngine->AddOnScreenDebugMessage(1, 15.f, FColor::Magenta, FString("Please switch to Single Player"));
		
		// Early return to prevent null pointer dereference and abort the new slot creation process since we cannot 
		// access save functionality without a valid FoxGameMode
		return;
	}
	
	// Assign the default map name from the game mode to the selected load slot, setting which level will be loaded when this save is started
	LoadSlots[Slot]->SetMapName(FoxGameMode->DefaultMapName);

	// Assign the player-entered name to the selected load slot's player name field
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	
	// Initialize the player level to 1 for this new save slot, representing a fresh game start at the beginning level
	LoadSlots[Slot]->SetPlayerLevel(1);
	
	// Mark the slot status as Taken to indicate this slot now contains valid save data
	LoadSlots[Slot]->SlotStatus = Taken;
	
	// Assign the default player start tag from the game mode to the load slot to determine which PlayerStart actor the 
	// player will spawn at when starting this new game
	LoadSlots[Slot]->PlayerStartTag = FoxGameMode->DefaultPlayerStartTag;
	
	// Extract and store the asset name from the default map's soft object path (e.g., "Map_Level1" from the full path) 
	// to identify which map asset this save slot should load
	LoadSlots[Slot]->MapAssetName = FoxGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();

	// Save the load slot data to persistent storage via the game mode's save system
	FoxGameMode->SaveSlotData(LoadSlots[Slot], Slot);

	// Initialize the slot to refresh its state and update its UI representation
	LoadSlots[Slot]->InitializeSlot();
	
	// Retrieve the game instance and cast it to UFoxGameInstance to access persistent game-wide data storage for save slot information
	UFoxGameInstance* FoxGameInstance = Cast<UFoxGameInstance>(FoxGameMode->GetGameInstance());

	// Store the selected load slot's name in the game instance so it persists across level transitions and can be used to load the correct save file
	FoxGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();

	// Store the selected load slot's index in the game instance for quick numeric identification of the save slot during level travel
	FoxGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;

	// Store the default player start tag in the game instance to determine which PlayerStart actor to spawn the player at when the new game begins
	FoxGameInstance->PlayerStartTag = FoxGameMode->DefaultPlayerStartTag;
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
	// Store the currently selected slot in the SelectedSlot member variable for later operations such as deletion or loading
	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	// Check if a valid slot is currently selected before attempting to delete it
	if (IsValid(SelectedSlot))
	{
		// Delete the save file from disk using the static DeleteSlot method with the slot's name and index
		AFoxGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);

		// Mark the slot status as Vacant to indicate this slot is now empty and available for new saves
		SelectedSlot->SlotStatus = Vacant;

		// Reinitialize the slot to refresh its state and update its UI representation to reflect the vacant status
		SelectedSlot->InitializeSlot();

		// Broadcast true to enable the select button for this slot since it's no longer selected after deletion
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	// Retrieve the current game mode and cast it to FoxGameModeBase to access level travel functionality
	AFoxGameModeBase* FoxGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	// Retrieve the game instance and cast it to UFoxGameInstance to access persistent game-wide data storage
	UFoxGameInstance* FoxGameInstance = Cast<UFoxGameInstance>(FoxGameMode->GetGameInstance());

	// Store the selected slot's player start tag in the game instance so it persists during level travel and determines 
	// which PlayerStart actor to spawn the player at in the loaded map
	FoxGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	
	// Store the selected slot's unique name identifier in the game instance so it persists across level transitions
	// and can be used to identify and load the correct save file when the player continues their game
	FoxGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();

	// Store the selected slot's numeric index in the game instance for quick identification of which save slot
	// (0, 1, or 2) is being loaded, used alongside LoadSlotName for save game operations
	FoxGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;

	/*
	 * Check if a valid slot is currently selected before attempting to load and travel to its associated map
	 * IsValid() is a global engine-defined function (not specific to any single class) that validates UObject pointers
	 * It checks if the pointer is not null, not pending kill, and references a valid object in memory
	 * This function is used throughout Unreal Engine code to safely validate any UObject-derived pointer before use
	 * In this case, we're checking if SelectedSlot (a UMVVM_LoadSlot pointer) is safe to dereference
	 */
	if (IsValid(SelectedSlot))
	{
		// Initiate level travel to the map associated with the selected slot using the game mode's TravelToMap method
		FoxGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	// Retrieve the current game mode and cast it to FoxGameModeBase to access save slot data functionality
	AFoxGameModeBase* FoxGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	// Early return guard: if the cast to FoxGameMode failed (e.g., wrong game mode type or nullptr), abort the load 
	// operation to prevent null pointer access
	if (!IsValid(FoxGameMode)) return;

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
		
		// Assign the loaded map name from the save object to the view model to track which level/map this save is associated with
		LoadSlot.Value->SetMapName(SaveObject->MapName);
		
		// Assign the loaded player start tag from the save object to the view model to determine which PlayerStart 
		// actor the player will spawn at when loading this save
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
		
		// Assign the loaded player level from the save object to the view model to track the character's progression level for this save slot
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
	} 
}

void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	// Update the NumLoadSlots property with the new value and automatically broadcast property changed notifications to bound UI elements via the MVVM framework
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
