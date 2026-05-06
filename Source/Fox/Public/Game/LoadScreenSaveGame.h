// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	
	// The unique name identifier for this save slot, used by Unreal's save game system to identify the save file
	UPROPERTY()
	FString SlotName = FString();

	// The numeric index of this save slot (0, 1, or 2), used to identify which of the three available slots this save belongs to
	UPROPERTY()
	int32 SlotIndex = 0;

	// The player's chosen name for their character, displayed in the load screen UI with a default value if not set
	UPROPERTY()
	FString PlayerName = FString("Default Name");
};
