// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "FoxAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	
	// Function to create a gameplay effect context
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
