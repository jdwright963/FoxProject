// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "FoxDamageGameplayAbility.h"
#include "FoxProjectileSpell.generated.h"

class AFoxProjectile;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class FOX_API UFoxProjectileSpell : public UFoxDamageGameplayAbility
{
	GENERATED_BODY()

protected:
	
	// Needs comment explaining all parameters and when this gets called
	// Override to handle activation of the projectile spell ability
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	// This function spawns the projectile for the spell, and it is called from the blueprint. ProjectileTargetLocation
	// is the location under the mouse cursor where the projectile should be spawned.
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0.f);
	
	// Projectile class to spawn for this spell. The value for this is set in the Blueprint and should be a valid 
	// subclass of AFoxProjectile.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AFoxProjectile> ProjectileClass;
	
	// Maximum number of projectiles that can be spawned when this spell ability is activated, allowing for multi-shot
	// projectile patterns or upgrades that increase projectile count
	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 5;
};
