// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxDamageGameplayAbility.h"
#include "FoxFireBlast.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxFireBlast : public UFoxDamageGameplayAbility
{
	GENERATED_BODY()
public:
	
	// Returns a formatted description string for the projectile spell ability at the specified level, using rich text
	// tags (<Default> and <Level>) to style the displayed text in the UI
	virtual FString GetDescription(int32 Level) override;

	// Returns a formatted string describing the benefits and changes gained at the next ability level, using rich text
	// tags for UI styling to help players understand progression
	virtual FString GetNextLevelDescription(int32 Level) override;
	
protected:
	
	// The number of fire balls spawned when this Fire Blast ability is activated
	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireBalls = 12;
};
