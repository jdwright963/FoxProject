// Copyright TryingToMakeGames


#include "Actor/FoxEnemySpawnPoint.h"

#include "Character/FoxEnemy.h"

void AFoxEnemySpawnPoint::SpawnEnemy()
{
	// Create spawn parameters to configure how the enemy actor will be spawned
	FActorSpawnParameters SpawnParams;
	
	// Se the collision handling policy on the spawn params to always spawn the enemy, adjusting position if needed to
	// avoid collision
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the enemy actor in a deferred state to allow property configuration before initialization
	AFoxEnemy* Enemy = GetWorld()->SpawnActorDeferred<AFoxEnemy>(EnemyClass, GetActorTransform());
	
	// Set the level of the enemy that will be spawned
	Enemy->SetLevel(EnemyLevel);
	
	// Set the enemy's character class (Warrior, Ranger, etc.)
	Enemy->SetCharacterClass(CharacterClass);
	
	// Complete the deferred spawning process by calling FinishSpawning. This finalizes the actor creation and runs
	// all initialization logic that was deferred, including BeginPlay, registering components, and applying the final
	// transform. We pass the spawn transform again to ensure the actor is placed at the correct location and rotation.
	// After this call, the enemy actor is fully initialized and active in the world.
	Enemy->FinishSpawning(GetActorTransform());
	
	// Create and assign the default AI controller to control the enemy
	Enemy->SpawnDefaultController();
}
