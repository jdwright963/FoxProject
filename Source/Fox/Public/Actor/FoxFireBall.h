// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Actor/FoxProjectile.h"
#include "FoxFireBall.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API AFoxFireBall : public AFoxProjectile
{
	GENERATED_BODY()
public:
	// Blueprint-implementable event that triggers the outgoing animation timeline for the fire ball.
	// This is called in BeginPlay() to start the initial movement/animation sequence when the fire ball is first spawned.
	// The timeline behavior (e.g., scaling, movement curves) should be defined in the Blueprint child class.
	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

	// Reference to the actor that this fire ball should return to after reaching its destination.
	// This is typically set when spawning fire balls in abilities like Fire Blast, where the projectiles
	// need to return to the caster after traveling outward in a radial pattern.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;
	
	// Damage parameters applied when the fire ball explodes on impact or at its destination.
	// This allows for separate damage configuration from the initial projectile hit, enabling
	// different explosion radius, damage values, and effects to be applied in an area.
	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageParams;
	
protected:
	virtual void BeginPlay() override;
	
	// Handles collision events when the fire ball's sphere component overlaps with another actor.
	// Validates the overlap, then applies damage effects with death impulse to valid targets on the server.
	// This override customizes the base projectile behavior to use the fire ball's specific damage parameters.
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	// Handles the fire ball's hit event by executing the FireBlast gameplay cue for explosion effects
	// (particles, sound, camera shake), stopping and destroying the looping flight sound component,
	// and marking the projectile as hit to prevent further collision processing.
	virtual void OnHit() override;
};
