// Copyright TryingToMakeGames


#include "Game/LoadScreenSaveGame.h"

FSavedMap ULoadScreenSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	// Iterate through all saved maps in the SavedMaps array to find a matching map
	for (const FSavedMap& Map : SavedMaps)
	{
		// Check if the current map's asset name matches the requested map name
		if (Map.MapAssetName == InMapName)
		{
			// Return the found map that matches the requested name
			return Map;
		}
	}
	// Return an empty/default FSavedMap if no matching map was found in the array
	return FSavedMap();
}

bool ULoadScreenSaveGame::HasMap(const FString& InMapName)
{
	// Iterate through all saved maps in the SavedMaps array to check if any map matches the requested name
	for (const FSavedMap& Map : SavedMaps)
	{
		// Check if the current map's asset name matches the requested map name
		if (Map.MapAssetName == InMapName)
		{
			// Return true immediately if a matching map is found, indicating the map exists in the saved maps collection
			return true;
		}
	}
	// Return false if no matching map was found after checking all maps in the array
	return false;
}
