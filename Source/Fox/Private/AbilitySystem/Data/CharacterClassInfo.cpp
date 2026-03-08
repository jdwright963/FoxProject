// Copyright TryingToMakeGames


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(ECharacterClass CharacterClass)
{
	// Searches the map for a key equal to the CharacterClass enum input parameter and returns the corresponding value
	// (FCharacterClassDefaultInfo struct)
	return CharacterClassInformation.FindChecked(CharacterClass);
}
