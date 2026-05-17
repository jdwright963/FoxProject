// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTiers.generated.h"

/**
 * Struct that defines a single loot item entry with spawn configuration settings.
 * This structure encapsulates all the necessary information for a loot item including what actor class to spawn,
 * its spawn probability, quantity limits, and level override behavior. Used by the loot tier system to configure
 * and manage randomized loot drops.
 */
USTRUCT(BlueprintType)
struct FLootItem
{
	GENERATED_BODY()

	// Variable that defines the actor class to spawn as loot. This determines what type of loot item will be created
	// when this entry is selected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootTiers|Spawning")
	TSubclassOf<AActor> LootClass;

	// Variable that represents the probability (0.0 to 1.0) of spawning this loot item. Higher values increase the
	// likelihood of this loot appearing
	UPROPERTY(EditAnywhere, Category = "LootTiers|Spawning")
	float ChanceToSpawn = 0.f;

	// Variable that defines the maximum number of instances of this loot item that can spawn.
	UPROPERTY(EditAnywhere, Category = "LootTiers|Spawning")
	int32 MaxNumberToSpawn = 0.f;

	// Variable that determines whether the loot uses a level override instead of the default level from the loot source
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LootTiers|Spawning")
	bool bLootLevelOverride = true;
};

/**
 * 
 */
UCLASS()
class FOX_API ULootTiers : public UDataAsset
{
	GENERATED_BODY()
public:
	
	// Function that returns the array of loot items configured for this loot tier. This allows external systems
	// to retrieve and process the loot configuration for spawning purposes
	UFUNCTION(BlueprintCallable)
	TArray<FLootItem> GetLootItems();

	// Variable that stores the array of loot item configurations for this in this data asset. Each entry defines a potential
	// loot item that can spawn, including its class, spawn chance, quantity limits, and level override settings
	UPROPERTY(EditDefaultsOnly, Category = "LootTiers|Spawning")
	TArray<FLootItem> LootItems;
};
