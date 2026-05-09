// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FoxGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	// The tag used to identify which PlayerStart actor the player should spawn at when loading into a level from a save game
	UPROPERTY()
	FName PlayerStartTag = FName();

	// The unique string identifier for the save slot (e.g., "LoadSlot_0") used when traveling to a map from the load screen
	UPROPERTY()
	FString LoadSlotName = FString();

	// The numeric index of the save slot (0, 1, or 2) used when traveling to a map from the load screen
	UPROPERTY()
	int32 LoadSlotIndex = 0;
};
