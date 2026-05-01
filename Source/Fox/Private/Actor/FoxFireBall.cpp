// Copyright TryingToMakeGames


#include "Actor/FoxFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"

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
