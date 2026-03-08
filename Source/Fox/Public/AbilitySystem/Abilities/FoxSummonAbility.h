// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxGameplayAbility.h"
#include "FoxSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxSummonAbility : public UFoxGameplayAbility
{
	GENERATED_BODY()
public:

	// Function that returns an array of spawn locations for minions
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();
	
	// Function that returns a random minion class from the MinionClasses array to be spawned.
	// BlueprintPure specifier means this function has no side effects (doesn't modify state) and can be called
	// in Blueprint without requiring an execution pin. Pure functions are typically used for calculations and
	// getters, appearing as simple nodes in Blueprint graphs that only have input/output data pins.
	UFUNCTION(BlueprintPure, Category="Summoning")
	TSubclassOf<APawn> GetRandomMinionClass();

	// Number of minions for this ability to summon. This, and the following variables can be set in the blueprint.
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	int32 NumMinions = 5;

	// Array of minion classes that this ability can summon
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	// Minimum distance to spawn minions away from the character summoning them
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MinSpawnDistance = 50.f;

	// Max distance to spawn minions away from the character summoning them
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MaxSpawnDistance = 250.f;

	// Angle in front of the character doing the summoning in which minions will be spawned. 
	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float SpawnSpread = 90.f;
};
