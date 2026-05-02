// Copyright TryingToMakeGames


#include "Actor/FoxFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "FoxGameplayTags.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"

void AFoxFireBall::BeginPlay()
{
	Super::BeginPlay();
	
	// Starts the outgoing animation timeline to play the fire ball's initial movement sequence
	StartOutgoingTimeline();
}

void AFoxFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Early return if the overlap is not valid (e.g., overlapping with self, instigator, or already hit target)
	if (!IsValidOverlap(OtherActor)) return;

	// Check if this actor has authority (is running on the server) to perform damage calculations
	if (HasAuthority())
	{
		// Attempt to get the Ability System Component from the overlapped actor to apply damage effects
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// Calculate the death impulse vector by multiplying the fireball's forward direction by the configured magnitude
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			
			// Assign the calculated death impulse to the damage effect parameters for ragdoll physics on death
			DamageEffectParams.DeathImpulse = DeathImpulse;

			// Set the target's Ability System Component in the damage parameters to identify who receives the damage
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			
			// Apply the damage effect to the target using the configured damage parameters
			UFoxAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

void AFoxFireBall::OnHit()
{
	// Verify that the fire ball has a valid owner (typically the ability caster) before attempting to execute
	// the gameplay cue, ensuring we have a valid context for the visual and audio effects
	if (GetOwner())
	{
		// Create a gameplay cue parameters structure to pass contextual information (like location) to the cue system
		FGameplayCueParameters CueParams;

		// Set the location parameter to the fire ball's current position so the explosion effect appears at the impact point
		CueParams.Location = GetActorLocation();

		// Execute the FireBlast gameplay cue non-replicated on the owner to trigger client-side explosion effects
		// (particles, sounds, camera shake) without network replication since this is called on each client locally
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FFoxGameplayTags::Get().GameplayCue_FireBlast, CueParams);
	}
	
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
	
	// Mark the projectile as having hit a target or surface, preventing further collision processing
	// and signaling that the projectile's lifecycle should transition to its destruction phase
	bHit = true;
}
