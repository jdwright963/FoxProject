// Copyright TryingToMakeGames


#include "AbilitySystem/Data/LootTiers.h"

TArray<FLootItem> ULootTiers::GetLootItems()
{
	// Create an array to store the loot items that will be spawned based on random chance calculations
	TArray<FLootItem> ReturnItems;

	// Iterate through each loot item definition in the LootItems array to determine which items should be spawned
	for (FLootItem& Item : LootItems)
	{
		// Attempt to spawn this item multiple times based on the MaxNumberToSpawn value, giving each attempt a chance to succeed
		for (int32 i = 0; i < Item.MaxNumberToSpawn; ++i)
		{
			// Generate a random number between 1 and 100 and check if it's less than the item's spawn chance percentage to determine if this spawn attempt succeeds
			if (FMath::FRandRange(1.f, 100.f) < Item.ChanceToSpawn)
			{
				// Create a new FLootItem instance that will be added to the return array if the spawn chance check passed
				FLootItem NewItem;
				
				// Copy the LootClass property from the source item to the new item being created
				NewItem.LootClass = Item.LootClass;

				// Copy the level override flag to determine whether this loot item should use a custom level instead of the default level
				NewItem.bLootLevelOverride = Item.bLootLevelOverride;
				
				// Add the newly created loot item to the return array since it successfully passed the spawn chance check
				ReturnItems.Add(NewItem);
			}
		}
	}
	// Return the array containing all loot items that successfully passed their spawn chance checks
	return ReturnItems;
}
