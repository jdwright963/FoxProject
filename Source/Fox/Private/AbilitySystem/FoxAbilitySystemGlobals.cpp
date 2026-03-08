// Copyright TryingToMakeGames


#include "AbilitySystem/FoxAbilitySystemGlobals.h"

#include "FoxAbilityTypes.h"

FGameplayEffectContext* UFoxAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	/**
	 * ALLOCATING A CUSTOM GAMEPLAY EFFECT CONTEXT
	 * 
	 * This function is called by the Ability System whenever a new FGameplayEffectContext needs to be created.
	 * By overriding this method in our custom UFoxAbilitySystemGlobals class, we ensure that the engine uses
	 * our extended FFoxGameplayEffectContext instead of the base FGameplayEffectContext.
	 * 
	 * WHY WE NEED THIS:
	 * The base FGameplayEffectContext doesn't have fields for tracking additional gameplay information like
	 * critical hits or blocked hits. Our custom FFoxGameplayEffectContext extends the base context to include:
	 * - bIsCriticalHit: Tracks whether a gameplay effect resulted from a critical hit
	 * - bIsBlockedHit: Tracks whether a gameplay effect was blocked
	 * 
	 * HOW IT WORKS:
	 * 1. The Ability System calls this function whenever it needs a new context (e.g., when applying a
	 *    gameplay effect, calculating damage, etc.)
	 * 2. We allocate our custom FFoxGameplayEffectContext using 'new', which creates it on the heap
	 * 3. The returned pointer is of type FGameplayEffectContext* (the base class), but it points to our
	 *    custom FFoxGameplayEffectContext object - this is polymorphism in action
	 * 4. The Ability System takes ownership of this allocated memory and will properly clean it up later
	 * 
	 * IMPORTANT NOTES:
	 * - This override must be registered in the project settings under "Ability System Globals Class Name"
	 *   to ensure the engine uses our custom UFoxAbilitySystemGlobals class. We did thi by putting this code in the
	 *   DefaultGame.ini config file:
	 * 
	 *		[/Script/GameplayAbilities.AbilitySystemGlobals]
	 *		+AbilitySystemGlobalsClassName="/Script/Fox.FoxAbilitySystemGlobals"
	 *		
	 * - The FFoxGameplayEffectContext includes proper NetSerialize() and Duplicate() implementations to
	 *   handle network replication and deep copying of our custom fields
	 * - Memory management is handled by the Ability System's smart pointer infrastructure, so we don't
	 *   need to manually delete the allocated context
	 */
	return new FFoxGameplayEffectContext();
}
