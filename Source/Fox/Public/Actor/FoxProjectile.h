// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "FoxAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "FoxProjectile.generated.h"

class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;

/**
 * The AFoxProjectile class is a subclass of AActor designed to represent a projectile in the game world.
 * It uses a sphere component to detect overlaps with other objects, which can be utilized for collision logic,
 * such as applying damage or triggering effects upon impact.
 *
 * Key Features:
 * - Utilizes a USphereComponent for overlap detection.
 * - Implements the OnSphereOverlap function to handle collision events.
 */
UCLASS()
class FOX_API AFoxProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoxProjectile();
	
	// Component that handles the movement logic for this projectile, including velocity, bounciness, and gravity.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	// Struct containing all the parameters needed to apply damage effects when the projectile hits a target.
	// This includes source/target ASC references, damage values, effect specs, and debuff information.
	// Exposed on spawn to allow customization per projectile instance (e.g., different damage types or magnitudes).
	//
	// The `meta = (ExposeOnSpawn)` specifier is a Blueprint metadata tag that makes this property visible
	// as an input pin when spawning this actor using Blueprint nodes like "Spawn Actor from Class".
	// Without this specifier, you would need to set this property after spawning the actor.
	// With ExposeOnSpawn:
	// - The property appears as a pin directly on the spawn node in Blueprint
	// - You can set the value at spawn time (instead of after), ensuring the projectile is fully configured from the start
	// - This is especially useful for properties that need to be set before BeginPlay() is called
	// - Commonly used for actor configuration that varies per instance (like damage params, targets, etc.)
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn))
	FDamageEffectParams DamageEffectParams;
	
	// Scene component reference used as the target for homing projectile behavior. When set, the projectile's
	// ProjectileMovementComponent will use this component's location to calculate homing trajectory adjustments.
	// This is typically assigned when spawning homing projectiles (e.g., in UFoxFireBolt::SpawnProjectiles) to
	// enable projectiles to track and follow moving targets throughout their flight path.
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/**
	 * Handles the visual and audio effects when the projectile hits something.
	 * This function is called on both server and client.
	 * 
	 * Responsibilities:
	 * - Plays the impact sound effect at the projectile's location
	 * - Spawns the Niagara impact particle effect
	 * - Stops the looping flight sound component
	 * - Sets the bHit flag to true to prevent duplicate effect execution
	 * 
	 * Note: This function does not handle damage application or projectile destruction,
	 * as those are managed separately in OnSphereOverlap() and Destroyed().
	 */
	void OnHit();

	// Overriden function called when the actor is destroyed
	virtual void Destroyed() override;
	
	/**
	 * Callback function triggered when the sphere component overlaps with another primitive component.
	 * The UFUNCTION macro exposes this function to Unreal's reflection system, allowing it to be bound
	 * as a delegate for overlap events (e.g., OnComponentBeginOverlap).
	 *
	 * @param OverlappedComponent - The primitive component on this actor that triggered the overlap (typically the Sphere component).
	 * @param OtherActor - The actor that is overlapping with this projectile's sphere component.
	 * @param OtherComp - The specific primitive component on the OtherActor that is causing the overlap.
	 * @param OtherBodyIndex - The index of the body that is overlapping, useful for components with multiple physics bodies.
	 * @param bFromSweep - True if the overlap occurred from a sweep (continuous collision detection), false if from simple overlap.
	 * @param SweepResult - Contains detailed information about the overlap/hit, including impact point, normal, and other collision data.
	 *                      This parameter is only valid when bFromSweep is true.
	 */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Sphere component to detect overlaps
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;
	
private:
	
	// Variable that specifies the lifespan of the projectile, after which the projectile will be destroyed.
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;
	
	// Variable to track if the projectile has hit something
	bool bHit = false;
	
	// Niagara system effect for impact of a projectile (this class)
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;
	
	// Sound effect for impact of a projectile (this class)
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;
	
	// Sound effect for projectile flying through the air. It plays until the projectile hits something
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
	
	// Audio component that will store the component created from LoopingSound
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;
};
