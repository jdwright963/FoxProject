// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "FoxGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	
	// Only used for startup abilities because other abilities might have their input tags changed at runtime. The value
	// for this variable is set in the blueprint
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;
	
	// Returns a formatted description string for the ability at the specified level
	virtual FString GetDescription(int32 Level);

	// Returns a formatted string describing the benefits of the next ability level
	virtual FString GetNextLevelDescription(int32 Level);

	// Returns a formatted string indicating the ability is locked until the required level
	static FString GetLockedDescription(int32 Level);
	
protected:

	// Retrieves the mana cost for this ability at the specified level by extracting the magnitude from the cost 
	// gameplay effect's mana attribute modifier, returns 0.f if no cost effect or mana modifier exists
	float GetManaCost(float InLevel = 1.f) const;

	// Retrieves the cooldown duration in seconds for this ability at the specified level by extracting the duration 
	// magnitude from the cooldown gameplay effect, returns 0.f if no cooldown effect exists
	float GetCooldown(float InLevel = 1.f) const;
};
