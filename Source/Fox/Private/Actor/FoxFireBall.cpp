// Copyright TryingToMakeGames


#include "Actor/FoxFireBall.h"

void AFoxFireBall::BeginPlay()
{
	Super::BeginPlay();
	
	// Starts the outgoing animation timeline to play the fire ball's initial movement sequence
	StartOutgoingTimeline();
}

void AFoxFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}
