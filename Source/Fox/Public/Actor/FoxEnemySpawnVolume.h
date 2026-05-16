// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SaveInterface.h"
#include "FoxEnemySpawnVolume.generated.h"

class UBoxComponent;
class AFoxEnemySpawnPoint;

UCLASS()
class FOX_API AFoxEnemySpawnVolume : public AActor, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoxEnemySpawnVolume();

	/* Save Interface */
	
	// Called when loading this actor's saved state from the save system. Restores the actor's state including whether 
	// the spawn volume has been reached.
	virtual void LoadActor_Implementation() override;
	
	/* end Save Interface */

	// Indicates whether the player has reached this spawn volume, triggering enemy spawns. This value is saved to 
	// persist spawn state across game sessions.
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Callback function triggered when an actor overlaps with the box component. When the player enters this volume,
	// it marks the volume as reached and triggers spawning of enemies at all configured spawn points.
	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Array of spawn points where enemies will be spawned when the player reaches this volume. Each spawn point 
	// defines the location, enemy class, level, and character class for the spawned enemy.
	UPROPERTY(EditAnywhere)
	TArray<AFoxEnemySpawnPoint*> SpawnPoints;

private:

	// The box component that defines the trigger volume. When the player overlaps this component, it marks the volume
	// as reached and triggers enemy spawning at all configured spawn points.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;

};
