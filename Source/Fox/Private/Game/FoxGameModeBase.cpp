// Copyright TryingToMakeGames


#include "Game/FoxGameModeBase.h"

#include "EngineUtils.h"
#include "Game/FoxGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
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
	
	// Store the player's current level from the load slot view model to track character progression
	LoadScreenSaveGame->PlayerLevel = LoadSlot->GetPlayerLevel();
	
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

ULoadScreenSaveGame* AFoxGameModeBase::RetrieveInGameSaveData()
{
	// Cast the game instance to our custom UFoxGameInstance type to access save slot information that was set when the player traveled to this map
	UFoxGameInstance* FoxGameInstance = Cast<UFoxGameInstance>(GetGameInstance());

	// Retrieve the unique slot name identifier (e.g., "LoadSlot_0") from the game instance that was stored during map travel
	const FString InGameLoadSlotName = FoxGameInstance->LoadSlotName;

	// Retrieve the numeric slot index (0, 1, or 2) from the game instance that was stored during map travel
	const int32 InGameLoadSlotIndex = FoxGameInstance->LoadSlotIndex;

	// Load and return the save game data associated with the current gameplay session using the slot name and index from the game instance
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void AFoxGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	// Cast the game instance to our custom UFoxGameInstance type to access the slot name and index that identify where to save the game data
	UFoxGameInstance* FoxGameInstance = Cast<UFoxGameInstance>(GetGameInstance());

	// Retrieve the unique slot name identifier (e.g., "LoadSlot_0") from the game instance that identifies which save file to write to
	const FString InGameLoadSlotName = FoxGameInstance->LoadSlotName;
	
	// Retrieve the numeric slot index (0, 1, or 2) from the game instance that identifies which save slot to write to
	const int32 InGameLoadSlotIndex = FoxGameInstance->LoadSlotIndex;
	
	// Update the game instance's PlayerStartTag with the one from the save object to ensure the correct spawn point is used when the player reloads the game
	FoxGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	// Write the updated save game object to persistent storage at the specified slot name and index, preserving all progress data
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void AFoxGameModeBase::SaveWorldState(UWorld* World)
{
	// Retrieve the current map's full name from the World object to identify which level's state we're saving
	FString WorldName = World->GetMapName();
	
	// Remove the streaming levels prefix from the map name to get the clean asset name used for save identification
	// StreamingLevelsPrefix is a string that Unreal Engine automatically prepends to map names when they are loaded as streaming levels (e.g., "/Game/Maps/")
	// This prefix needs to be removed because we want to store only the base asset name (e.g., "MainLevel" instead of "/Game/Maps/MainLevel")
	// in our save data structure for consistent identification and comparison when loading saved game states across different sessions
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	// Cast the game instance to UFoxGameInstance to access the LoadSlotName and LoadSlotIndex needed for save operations
	UFoxGameInstance* FoxGI = Cast<UFoxGameInstance>(GetGameInstance());
	
	// Validate that the cast succeeded; if FoxGI is null, this will crash the program to catch configuration errors early
	check(FoxGI);

	// Load the save game data for the current slot and enter this block if the load was successful
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(FoxGI->LoadSlotName, FoxGI->LoadSlotIndex))
	{
		// Check if this world/map has not been saved before in the save game data
		if (!SaveGame->HasMap(WorldName))
		{
			// Create a new FSavedMap structure to hold the state data for this previously unsaved map
			FSavedMap NewSavedMap;
			
			// Store the clean map asset name in the new saved map structure for identification when loading
			NewSavedMap.MapAssetName = WorldName;
			
			// Add the new saved map structure to the array of saved maps in the save game object
			SaveGame->SavedMaps.Add(NewSavedMap);
		}
		// Retrieve the saved map data structure for the current world from the save game object using the world's asset name
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);

		// Clear all previously saved actor data from this map's saved actors array to prepare for fresh serialization of current world state
		SavedMap.SavedActors.Empty(); 
		
		// Iterate through all actors currently present in the world using FActorIterator to find actors that need to be saved
		// FActorIterator is an Unreal Engine helper class that provides a convenient way to iterate over all actors in a world
		// The for loop pattern works as follows:
		//   - FActorIterator It(World): This calls the FActorIterator constructor to create an iterator object named 'It',
		//     passing the World pointer as a parameter to initialize the iterator with the given world, starting at the first actor.
		//     This is standard C++ Direct Initialization. It is exactly the same as writing: FActorIterator It = FActorIterator(World);
		//   - It: Checks if the iterator is valid (returns true if there are more actors to process, false when done)
		//   - ++It: Advances the iterator to the next actor in the world after each loop iteration
		for (FActorIterator It(World); It; ++It)
		{
			// Dereference the actor iterator using the * operator to obtain the actual AActor pointer for the current actor being processed
			// Dereferencing is the process of accessing the value that a pointer or iterator points to
			// FActorIterator overloads the * operator to return the AActor* that the iterator currently points to
			// This converts the iterator object into the actual actor pointer we can work with
			// Similar to dereferencing a raw pointer: if you have AActor* ptr, then *ptr gives you the AActor object itself
			// Here, *It calls FActorIterator::operator*() which returns the AActor* at the iterator's current position
			AActor* Actor = *It;

			// Skip this actor if it's invalid or doesn't implement the USaveInterface, ensuring only saveable actors are processed
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			// Create a new FSavedActor structure to hold this actor's serialized state data (name, transform, and serialized properties)
			FSavedActor SavedActor;

			// Store the actor's unique FName identifier to enable finding and restoring the correct actor when loading the save
			SavedActor.ActorName = Actor->GetFName();

			// Store the actor's current world transform (position, rotation, and scale) to restore its spatial state when loading
			SavedActor.Transform = Actor->GetTransform();
			
			// Create a memory writer archive that will serialize the actor's property data into the SavedActor.Bytes TArray<uint8> for persistent storage
			// FMemoryWriter is an Unreal Engine archive class that writes serialized data directly into a byte array in memory
			// instead of writing to a file on disk. It implements the FArchive interface and provides functionality to convert
			// C++ objects and their properties into a linear sequence of bytes that can be stored in the SavedActor.Bytes array.
			// This byte array can then be saved to disk as part of the save game file and later deserialized to restore the actor's state.
			// The MemoryWriter will be wrapped by the proxy archive below to handle special serialization requirements for UObject references.
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			// Wrap the memory writer in an archive that serializes UObject references and FNames as strings.
			// This is useful for save data because raw UObject pointers are only memory addresses and are not valid
			// after the game exits or the level is reloaded.
			//
			// FObjectAndNameAsStringProxyArchive serializes UObject references by name/path and serializes FNames
			// as strings, allowing those references to be resolved again when loading if the objects can be found
			// or loaded.
			//
			// The second parameter, bInLoadIfFindFails, only matters when this archive is used for loading.
			// If true, unresolved object references may be loaded from disk if they cannot be found in memory.
			// Since this archive currently wraps an FMemoryWriter, the flag has no effect during saving, but using
			// true is common when the same archive pattern is also used for loading save data.
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			
			// Set the archive's save game flag to true to mark this as a save game archive, which tells Unreal Engine 
			// to use SaveGame-specific serialization behavior for properties marked with the SaveGame specifier.
			//
			// When ArIsSaveGame is true, the Serialize() method will only serialize UPROPERTY members that have the 
			// SaveGame specifier. This is critical for save systems because it provides fine-grained control over 
			// which properties are persisted to disk versus which are runtime-only or derived data.
			//
			// For example, in an actor class you might have:
			//   UPROPERTY(SaveGame) int32 Health;           // This WILL be serialized when ArIsSaveGame = true
			//   UPROPERTY() int32 CachedDamageValue;        // This will NOT be serialized when ArIsSaveGame = true
			//
			// Without this flag set to true, calling Serialize() would serialize ALL properties regardless of the 
			// SaveGame specifier, which could bloat save files with unnecessary data.
			//
			// This flag is part of Unreal's FArchive base class and is checked internally by the engine's serialization 
			// code when determining whether to serialize each individual property during the Serialize() call below.
			Archive.ArIsSaveGame = true;

			// Serialize the actor's properties (those marked with SaveGame specifier) into the archive's byte array 
			// for persistent storage, capturing the actor's current state data
			Actor->Serialize(Archive);

			// Add the serialized actor data to the saved actors array, using AddUnique to prevent duplicate entries 
			// if the actor was already saved in this map's state
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		// Iterate through all saved maps in the save game object to find and replace the map entry that matches 
		// the current world name with the newly serialized world state
		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			// Check if the current saved map's asset name matches the world name we're trying to update
			if (MapToReplace.MapAssetName == WorldName)
			{
				// Assign the updated SavedMap to replace the old map data with the new serialized world state containing
				// all current actor data
				MapToReplace = SavedMap;
			}
		}
		// Write the updated save game object to persistent storage at the specified slot, preserving all changes made 
		// during this save operation including the updated world state
		UGameplayStatics::SaveGameToSlot(SaveGame, FoxGI->LoadSlotName, FoxGI->LoadSlotIndex);
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
