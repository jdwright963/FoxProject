// Copyright TryingToMakeGames


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	
	// Start the search for the character's current level at level 1 (minimum character level)
	int32 Level = 1;
	
	// Creates a boolean variable and sets its value to true.
	bool bSearching = true;
	
	// Loop until bSearching is false
	while (bSearching)
	{
		// Prevent array out-of-bounds by capping at the maximum configured level (`LevelUpInformation.Num() - 1`).
		// If Level equals or exceeds the highest valid index in LevelUpInformation array, return the current level.
		// LevelUpInformation[0] = No Level Information, index 0 is a placeholder so that index 1 is the first level
		// LevelUpInformation[1] = Level 1 Information
		// LevelUpInformation[2] = Level 2 Information
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		// Check if the player's current XP meets or exceeds the requirement for advancing to the NEXT level.
		// LevelUpInformation[Level].LevelUpRequirement contains the XP threshold needed to advance FROM
		// Level to Level+1. If the player's XP is greater than or equal to this threshold, they qualify
		// to advance to the next level, so we should continue checking if they qualify for even higher levels.
		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			// Player has enough XP to advance to the next level, so check if they can advance further
			// by incrementing Level
			++Level;
		}
		else
		{
			// Player doesn't have enough XP to advance to the next level, so stop searching
			bSearching = false;
		}
	}
	// Return the current level. This is correct because LevelUpInformation[Level].LevelUpRequirement
	// represents the XP needed to reach Level+1, NOT Level itself. When we exit the loop, it means
	// the player doesn't have enough XP to reach Level+1, so Level is their actual current level.
	return Level;
}
