// Copyright TryingToMakeGames


#include "Actor/FoxProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Fox/Fox.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AFoxProjectile::AFoxProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Make the projectile class instance replicate to clients
	bReplicates = true;
	
	// Construct sphere component
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	
	// Make the sphere the root component
	SetRootComponent(Sphere);
	
	// Set collision object type to projectile
	Sphere->SetCollisionObjectType(ECC_Projectile);
	
	/*
	 * Configure collision settings for the sphere component to detect overlaps with specific object types.
	 * 
	 * Line 1: Enable collision for overlap queries only (no physics simulation).
	 *         QueryOnly means the sphere can detect overlaps but won't physically block or simulate physics.
	 * 
	 * Line 2: Set default collision response to ignore all channels initially.
	 *         This creates a clean slate before selectively enabling specific channels.
	 * 
	 * Line 3: Enable overlap response for WorldDynamic objects (movable physics objects in the world).
	 *         This allows the projectile to detect collisions with dynamic environmental objects.
	 * 
	 * Line 4: Enable overlap response for WorldStatic objects (static geometry like walls and floors).
	 *         This allows the projectile to detect collisions with level geometry and static meshes.
	 * 
	 * Line 5: Enable overlap response for Pawn objects (player characters and AI-controlled entities).
	 *         This allows the projectile to detect collisions with characters in the game.
	 */
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// Construct the projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	
	// Set initial projectile properties including setting no gravity
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AFoxProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Sets the lifespan of the projectile
	SetLifeSpan(LifeSpan);
	
	// Enable automatic replication of this projectile's movement to all clients so they see smooth synchronized motion.
	// This must be called in BeginPlay() rather than the constructor because the replication system is not fully 
	// initialized during construction. The actor's network role, owner, and connection state are only properly set 
	// after the actor is spawned and added to the world, which happens before BeginPlay() but after the constructor.
	SetReplicateMovement(true);
	
	// Prevent immediate self-overlap on server/client regardless of effect context state
	if (AActor* MyOwner = GetOwner())
	{
		Sphere->IgnoreActorWhenMoving(MyOwner, true);
	}
	if (APawn* MyInstigator = GetInstigator())
	{
		Sphere->IgnoreActorWhenMoving(MyInstigator, true);
	}
	
	// Bind our callback function OnSphereOverlap to the sphere components engine defined delegate OnComponentBeginOverlap
	// so that when the projectile collides with another actor causing the delegate to broadcast, the OnSphereOverlap function will be called
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AFoxProjectile::OnSphereOverlap);
	
	// Creates a component that does not live on an actor
	// Attaches the looping sound component to the root component of the projectile
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}


void AFoxProjectile::OnHit()
{
	// Plays the ImpactSound at the location of this actor on overlap. A zero rotator is used because it does not matter
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	
	// Spawns the niagara system at the location of this actor on overlap
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	
	// Check if the looping sound component exists before attempting to stop and destroy it
	// to prevent null pointer access and ensure safe cleanup of the audio component
	if (LoopingSoundComponent)
	{
		// Stop the looping projectile sound immediately to prevent it from continuing to play
		// after the projectile has hit its target and is about to be destroyed
		LoopingSoundComponent->Stop();

		// Destroy the audio component to free up memory and remove it from the actor's component list,
		// ensuring proper cleanup of resources associated with the projectile's flight sound
		LoopingSoundComponent->DestroyComponent();
	}
		
	// Set bHit to true to prevent impact effects from playing multiple times if OnSphereOverlap is called again
	// before the projectile is destroyed (e.g., overlapping multiple colliders simultaneously or rapid overlaps)
	bHit = true;
}

void AFoxProjectile::Destroyed()
{
	// Check if the looping sound component exists before attempting to stop and destroy it
	// to prevent null pointer access and ensure safe cleanup of the audio component. We do this here in addition to 
	// the OnHit callback to ensure the sound is stopped and cleaned up even if the projectile is destroyed without 
	// hitting anything. This would occur if it does not hit anything before its lifespan expires.
	if (LoopingSoundComponent)
	{
		// Stop the looping projectile sound immediately to prevent it from continuing to play
		// after the projectile has hit its target and is about to be destroyed
		LoopingSoundComponent->Stop();
		
		// Destroy the audio component to free up memory and remove it from the actor's component list,
		// ensuring proper cleanup of resources associated with the projectile's flight sound
		LoopingSoundComponent->DestroyComponent();
	}
	
	// Check if we're on a client (!HasAuthority) and the projectile hasn't registered a hit yet (!bHit).
	// This handles the edge case where the server destroys the projectile and replicates that destruction
	// to the client before the client's own OnSphereOverlap callback has a chance to execute, which would
	// result in missing impact effects on the client side.
	if (!bHit && !HasAuthority())
	{
		// Manually trigger impact effects on the client to ensure they play even when the server
		// replicates destruction before the client's OnSphereOverlap callback fires
		OnHit();
	}
	
	// Parent class code that destroys the actor
	Super::Destroyed();
}

void AFoxProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Early exit if the source Ability System Component is invalid. This prevents null pointer crashes
	// when trying to access the source ASC later for damage application.
	if (DamageEffectParams.SourceAbilitySystemComponent == nullptr) return;
	
	// Retrieve the avatar actor (typically the character) that owns the source Ability System Component.
	// This is the actor that originally cast the ability and spawned this projectile, extracted from
	// the DamageEffectParams struct that was configured when the projectile was created.
	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	// Prevent self-damage by checking if the source actor (who cast the ability) is the same as the actor
	// we just overlapped with. This ensures actors cannot damage themselves with their own projectiles.
	if (SourceAvatarActor == OtherActor) return;

	// Prevent friendly fire by using a custom library function we created to check if the source actor and the
	// overlapped actor are friends/allies. If they are friends (IsNotFriend returns false), exit early
	// without applying damage, allowing team-based gameplay where allies don't hurt each other.
	if (!UFoxAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;
	
	// Only trigger impact effects if this is the first valid hit. The bHit flag prevents duplicate effect execution
	// if OnSphereOverlap is called multiple times (e.g., overlapping multiple colliders simultaneously or network edge cases).
	// Once bHit is set to true by OnHit(), this block won't execute again for this projectile instance.
	if (!bHit)
	{
		OnHit();
	}
	// Server-authoritative damage application and projectile cleanup.
	// Only the server (HasAuthority() == true) should apply gameplay effects and destroy the projectile
	// to ensure consistent game state across all clients. The server's destruction will automatically
	// replicate to all clients, triggering their Destroyed() function if needed.
	if (HasAuthority())
	{
		// Retrieve the Ability System Component from the actor we just overlapped with (OtherActor).
		// This ASC is required to apply gameplay effects (like damage) to the target actor.
		// If OtherActor doesn't have an ASC, this returns nullptr and the damage application is skipped.
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// Calculate the death impulse vector by multiplying the projectile's forward direction with the configured magnitude.
			// This creates a force vector that will be applied to the target's physics body upon death, pushing them away
			// in the direction the projectile was traveling for a more dynamic death effect (e.g., ragdoll knockback).
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;

			// Store the calculated death impulse in DamageEffectParams so it can be accessed during damage application.
			// If the damage kills the target, this impulse vector will be applied to their physics body to create
			// a satisfying knockback effect in the direction the projectile was moving.
			DamageEffectParams.DeathImpulse = DeathImpulse;
			
			// Perform a random roll (1-100) to determine if knockback should be applied to the target.
			// If the random number is less than the configured KnockbackChance percentage, knockback will occur.
			// For example, if KnockbackChance is 30, there's a 30% chance this will be true.
			const bool bKnockback = FMath::RandRange(1, 100) < DamageEffectParams.KnockbackChance;

			// If the knockback chance roll succeeded, calculate and apply the knockback force to the target.
			if (bKnockback)
			{
				// Get the current rotation of the projectile to use as the base for knockback direction.
				// We retrieve this rotation so we can modify it to create an upward-angled knockback effect.
				FRotator Rotation = GetActorRotation();

				// Set the pitch (up and down) to 45 degrees to create an upward-angled knockback trajectory.
				// This makes knocked-back targets launch into the air rather than just pushing horizontally,
				// creating a more satisfying and visible combat effect.
				Rotation.Pitch = 45.f;

				// Convert the modified rotation (now angled upward at 45 degrees) into a direction vector.
				// This vector represents the direction the target will be knocked back, combining forward
				// momentum with upward lift
				const FVector KnockbackDirection = Rotation.Vector();

				// Calculate the final knockback force by scaling the direction vector with the configured magnitude.
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
				
				// Then store the KnockbackForce in DamageEffectParams so it can be accessed when applying the damage 
				// effect to the target.
				DamageEffectParams.KnockbackForce = KnockbackForce;
			}
			
			// Assign the target's ASC to our DamageEffectParams struct so it knows which actor to apply damage to.
			// DamageEffectParams already contains the source ASC, damage values, and effect specifications set when the projectile was spawned.
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;

			// Apply the damage effect to the target using our custom library function.
			// This function uses the configured DamageEffectParams to create and apply a gameplay effect spec
			// that handles damage calculation, resistance, debuffs, and other combat mechanics on the target actor.
			UFoxAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		// Very unlikely that this could replicate down to the client before the client has had its OnSphereOverlap
		// function called destroying the projectile before playing the effects. Just in case we do the following else
		// statement and then override Destroyed() and play the effects if it gets called before OnSphereOverlap on the client
		Destroy();
	}
	else
	{
		// Client-side hit acknowledgment. When OnSphereOverlap executes on a client (!HasAuthority()),
		// we set bHit to true to indicate the client has processed the overlap and played its impact effects.
		// This flag works with the Destroyed() override to handle the edge case where the server's destruction
		// replicates to the client before the client's OnSphereOverlap callback executes.
		bHit = true;
	}
}

