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

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
