// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxProjectileSpell.h"
#include "FoxFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxFireBolt : public UFoxProjectileSpell
{
	GENERATED_BODY()
public:
	
	// Returns a formatted description string for the projectile spell ability at the specified level, using rich text
	// tags (<Default> and <Level>) to style the displayed text in the UI
	virtual FString GetDescription(int32 Level) override;

	// Returns a formatted string describing the benefits and changes gained at the next ability level, using rich text
	// tags for UI styling to help players understand progression
	virtual FString GetNextLevelDescription(int32 Level) override;
	
	// Spawns multiple projectiles in a spread pattern towards the target location. This function has an 's' at the end
	// of its name, which distinguishes it from the parent class's SpawnProjectile function. Note: If two functions have
	// the same name but different parameters, it would be function overloading, not overriding (which requires matching
	// signatures in inheritance hierarchies). This function has an additional parameter for homing behavior.
	// @param ProjectileTargetLocation - The world location where projectiles should be aimed
	// @param SocketTag - Gameplay tag identifying which socket to spawn projectiles from
	// @param bOverridePitch - Whether to use a custom pitch value instead of calculated pitch
	// @param PitchOverride - Custom pitch angle to use when bOverridePitch is true
	// @param HomingTarget - Optional target actor for projectiles to home in on
	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget);

protected:

	// The maximum angle in degrees across which multiple projectiles will be distributed, creating a spread pattern
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float ProjectileSpread = 90.f;

	// The maximum number of projectiles that can be spawned simultaneously when this ability is activated
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	int32 MaxNumProjectiles = 5;
	
	// The minimum acceleration magnitude (in units/second²) that homing projectiles will use to track their target.
	// This value is applied to the ProjectileMovementComponent's HomingAccelerationMagnitude when spawning projectiles
	// with homing behavior enabled. Lower values result in wider, slower-turning projectile arcs that less
	// aggressively pursue targets.
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float HomingAccelerationMin = 1600.f;

	// The maximum acceleration magnitude (in units/second²) that homing projectiles will use to track their target.
	// This value is applied to the ProjectileMovementComponent's HomingAccelerationMagnitude when spawning projectiles
	// with homing behavior enabled. Higher values result in tighter, faster-turning projectile arcs that more
	// aggressively pursue targets.
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	float HomingAccelerationMax = 3200.f;

	// Determines whether spawned projectiles should actively track and follow their HomingTarget using the
	// ProjectileMovementComponent's homing functionality. When true, projectiles will adjust their trajectory
	// to pursue the target. When false, projectiles fly in a straight line toward their initial direction.
	UPROPERTY(EditDefaultsOnly, Category = "FireBolt")
	bool bLaunchHomingProjectiles = true;
};
