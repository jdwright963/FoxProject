// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Engine/TargetPoint.h"
#include "FoxEnemySpawnPoint.generated.h"

class AFoxEnemy;
/**
 * 
 */
UCLASS()
class FOX_API AFoxEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
public:
	
	/** Spawns an enemy at this spawn point's location with the configured class, level, and character class. */
	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

	/** The class of enemy to spawn at this point. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	TSubclassOf<AFoxEnemy> EnemyClass;

	/** The level of the enemy to spawn, determining its stats and difficulty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	int32 EnemyLevel = 1;

	/** The character class of the enemy (Warrior, Ranger, etc.), defining its abilities and playstyle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
};
