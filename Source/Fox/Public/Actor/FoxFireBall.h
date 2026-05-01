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
	
protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
