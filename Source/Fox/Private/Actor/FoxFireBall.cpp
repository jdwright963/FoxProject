// Copyright TryingToMakeGames


#include "Actor/FoxFireBall.h"

void AFoxFireBall::BeginPlay()
{
	Super::BeginPlay();
}

void AFoxFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}
