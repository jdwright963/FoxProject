// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

/**
 * Represents the configuration data for a single character level.
 * Contains experience requirements and reward information for progression to a specific level.
 * The values of the variables of this struct are set in the editor
 */
USTRUCT(BlueprintType)
struct FFoxLevelUpInfo
{
	GENERATED_BODY()
	
	// The amount of experience points required to reach this level
	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	// The number of attribute points awarded to the player when reaching this level
	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointAward = 1;

	// The number of spell points awarded to the player when reaching this level
	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointAward = 1;
};

/**
 * 
 */
UCLASS()
class FOX_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	
	// Array of instances of the FFoxLevelUpInfo struct that is defined above that defines XP requirements and rewards 
	// for each character level. Configured in the editor to set up the entire leveling curve for the game.
	// Index corresponds to level number (e.g., LevelUpInformation[1] = Level 1 data).
	UPROPERTY(EditDefaultsOnly)
	TArray<FFoxLevelUpInfo> LevelUpInformation;

	// Calculates and returns the character level based on total accumulated experience points.
	// @param XP - The total experience points to evaluate
	// @return The character level corresponding to the given XP amount
	int32 FindLevelForXP(int32 XP) const;
};
