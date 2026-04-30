// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxDamageGameplayAbility.h"
#include "ArcaneShards.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UArcaneShards : public UFoxDamageGameplayAbility
{
	GENERATED_BODY()
public:
	
	// Returns a formatted description string for the ability at the specified level, using rich text
	// tags (<Default> and <Level>) to style the displayed text in the UI
	virtual FString GetDescription(int32 Level) override;

	// Returns a formatted string describing the benefits and changes gained at the next ability level, using rich text
	// tags for UI styling to help players understand progression
	virtual FString GetNextLevelDescription(int32 Level) override;
	
	// The maximum number of arcane shards that can be spawned simultaneously when this ability is activated
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumShards = 11;
};
