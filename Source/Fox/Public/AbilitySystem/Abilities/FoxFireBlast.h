// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxDamageGameplayAbility.h"
#include "FoxFireBlast.generated.h"

class AFoxFireBall;
/**
 * 
 */
UCLASS()
class FOX_API UFoxFireBlast : public UFoxDamageGameplayAbility
{
	GENERATED_BODY()
public:
	
	// Returns a formatted description string for the projectile spell ability at the specified level, using rich text
	// tags (<Default> and <Level>) to style the displayed text in the UI
	virtual FString GetDescription(int32 Level) override;

	// Returns a formatted string describing the benefits and changes gained at the next ability level, using rich text
	// tags for UI styling to help players understand progression
	virtual FString GetNextLevelDescription(int32 Level) override;
	
	// Spawns and returns an array of fire ball projectiles in a circular pattern around the ability owner, with the
	// number of projectiles determined by NumFireBalls variable
	UFUNCTION(BlueprintCallable)
	TArray<AFoxFireBall*> SpawnFireBalls();
	
protected:
	
	// The number of fire balls spawned when this Fire Blast ability is activated
	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireBalls = 12;
	
private:

	// The class of fire ball projectile to spawn when this ability is activated. The value of this variable is set in the blueprint.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFoxFireBall> FireBallClass;
};
