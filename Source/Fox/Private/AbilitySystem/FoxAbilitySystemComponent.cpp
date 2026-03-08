// Copyright TryingToMakeGames


#include "AbilitySystem/FoxAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/FoxGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Fox/FoxLogChannels.h"
#include "Interaction/PlayerInterface.h"

void UFoxAbilitySystemComponent::AbilityActorInfoSet()
{
	/*
	 * Bind the OnGameplayEffectAppliedDelegateToSelf delegate to this component's EffectApplied callback.
	 * This delegate is broadcast whenever a gameplay effect is successfully applied to this ability system component.
	 * The EffectApplied callback will be invoked with details about the applied effect, allowing us to respond
	 * to gameplay effects (such as damage, buffs, debuffs) as they are applied to this actor.
	 */
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UFoxAbilitySystemComponent::ClientEffectApplied);
}

void UFoxAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	// Loop through each UGameplayAbility class in the StartupAbilities TArray input parameter
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		// Create ability spec
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		
		// Cast the ability from the AbilitySpec to UFoxGameplayAbility and check if the cast is successful
		if (const UFoxGameplayAbility* FoxAbility = Cast<UFoxGameplayAbility>(AbilitySpec.Ability))
		{
			
			/*
			 * Add the ability's StartupInputTag to the ability spec's dynamic tags container.
			 * 
			 * We use GetDynamicSpecSourceTags() instead of the deprecated DynamicAbilityTags member variable.
			 * GetDynamicSpecSourceTags() returns a FGameplayTagContainer that holds tags associated with this 
			 * specific ability instance. These tags can be queried at runtime to identify and filter abilities 
			 * (e.g., finding which ability corresponds to a specific input action).
			 * 
			 * StartupInputTag is a FGameplayTag defined in the UFoxGameplayAbility blueprint that identifies which input
			 * action should trigger this ability (e.g., "InputTag.LMB" for left mouse button or "InputTag.1" for the 1 key).
			 * 
			 * By adding the StartupInputTag to the dynamic tags, we create a binding between the input system and this
			 * ability. When the player presses an input (handled in AFoxPlayerController::AbilityInputTagPressed/Held/Released),
			 * the input's gameplay tag can be used to search through all granted abilities and find the one with a matching
			 * tag in its dynamic tags, allowing the ability system to activate the correct ability.
			 * 
			 * This approach provides a flexible, data-driven way to map inputs to abilities without hardcoding specific
			 * ability classes to specific input actions.
			 */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FoxAbility->StartupInputTag);
			
			/*
			 * Mark this ability as equipped by adding the Abilities_Status_Equipped tag to its dynamic tags.
			 * This status tag allows other systems to query whether an ability is currently equipped and usable,
			 * as opposed to being learned but unequipped (stored in an ability inventory). When abilities are
			 * unequipped or swapped, this tag can be removed and added to different ability specs to track
			 * which abilities the player currently has equipped for use.
			 */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FFoxGameplayTags::Get().Abilities_Status_Equipped);
			
			// Grant the ability to the owner of this ASC but do not activate it
			GiveAbility(AbilitySpec);
		}
	}
		/*
		 * Set the flag to true indicating that startup abilities have been successfully granted.
		 * This flag is used to prevent duplicate ability grants if AddCharacterAbilities is called multiple times,
		 * and signals to other systems that this ASC is fully initialized and ready to use its abilities.
		 */
		bStartupAbilitiesGiven = true;
	
		/*
		 * Broadcast the AbilitiesGivenDelegate multicast delegate.
		 * This notifies all bound listeners (such as UI widgets or game systems) that abilities have been granted
		 * and are now available. Listeners can use this notification to update UI elements like ability bars,
		 * initialize ability-related systems, or perform any other logic that depends on abilities being ready.
		 */
		AbilitiesGivenDelegate.Broadcast();
}

void UFoxAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	// Loop through the StartupPassiveAbilities array
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		// Create a gameplay ability spec for the current passive ability in the loop and use level 1, since this effect
		// does not scale or change with level
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		
		/*
		 * Grant the passive ability to this ASC and immediately activate it once.
		 * GiveAbilityAndActivateOnce() is a convenience function that combines two operations:
		 * 1. GiveAbility() - Grants the ability to this ASC, adding it to the list of activatable abilities
		 * 2. TryActivateAbility() - Immediately attempts to activate the newly granted ability one time
		 * 
		 * This function is specifically designed for passive abilities that need to execute their logic once
		 * when granted (such as applying permanent stat buffs or initializing passive effects) rather than
		 * waiting for player input. Unlike regular abilities that are activated through input actions,
		 * passive abilities typically don't need input tags and should activate automatically.
		 * 
		 * After activation, the ability remains granted to this ASC and can potentially be activated again
		 * if needed, though most passive abilities are designed to run once and apply their effects through
		 * gameplay effects that persist for the duration specified in their configuration.
		*/
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UFoxAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	// Check if the InputTag is not a valid tag that exists in this project. If so, return early.
	if (!InputTag.IsValid()) return;

	// Lock the ActivatableAbilities container until the for loop is done.
	FScopedAbilityListLock ScopedAbilityListLock(*this);
	
	// Loop through the activatable abilities specs that this ASC has
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		/*
		 * Check if this ability spec's dynamic tags contain an exact match for the InputTag.
		 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer that holds this ability spec's dynamic tags
		 * (which includes the StartupInputTag we added in AddCharacterAbilities).
		 * HasTagExact() performs an exact match check. It returns true only if the InputTag exactly matches one of
		 * the tags in the container (not just a parent tag match).
		 */
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			/*
			 * Notify the ability spec that its associated input has been pressed.
			 * This is an internal bookkeeping function required by Unreal's ability system that marks the spec as
			 * having received input. This state is used by the ability system for various checks and may trigger
			 * ability-specific logic that responds to input press events.
			 */
			AbilitySpecInputPressed(AbilitySpec);
			
			/*
			 * Check if the ability is currently active (running).
			 * IsActive() returns false if the ability is not currently executing, meaning it's safe to activate it.
			 * Some abilities can be activated multiple times simultaneously (if configured), but this check ensures
			 * we only try to activate inactive abilities, preventing unwanted duplicate activations for abilities
			 * that should only run once at a time.
			 */
			if (AbilitySpec.IsActive())
			{
				/*
				 * Retrieve the primary instance of the ability for InstancedPerActor abilities.
				 * GetPrimaryInstance() returns a pointer to the main UGameplayAbility instance that was created when this
				 * ability was first activated. This function is only valid for abilities with the InstancedPerActor instancing
				 * policy, which creates one ability instance per actor that is reused across multiple activations. For abilities
				 * with other instancing policies (InstancedPerExecution, NonInstanced), this would return nullptr.
				 */
				UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance();

				/*
				 * Check if the primary instance pointer is valid before attempting to invoke the replicated event.
				 * This null check is necessary because GetPrimaryInstance() may return nullptr if called on an ability
				 * with an incompatible instancing policy, or if the ability instance hasn't been created yet due to timing
				 * or initialization issues. Without this check, attempting to call GetCurrentActivationInfo() on a null pointer
				 * would cause a crash. This safety guard ensures we only notify abilities that have valid instances.
				 */
				if (PrimaryInstance)
				{
					/* Do not Delete or simplify!!! This is profound and long for a reason.
					 * Notify the ability instance that its input has been pressed by invoking a replicated event.
					 * InvokeReplicatedEvent() is a GAS function that sends an ability event across the network in multiplayer games,
					 * ensuring both server and clients are synchronized about input state. We pass EAbilityGenericReplicatedEvent::InputPressed
					 * to indicate this is an input press event, AbilitySpec.Handle to identify which ability received the input,
					 * and the activation prediction key from GetCurrentActivationInfo().
					 * 
					 * The activation prediction key is critical for client-side prediction and server reconciliation. In networked
					 * gameplay, clients predict ability activations locally for responsive gameplay without waiting for server confirmation.
					 * Each predicted action gets a unique prediction key that identifies that specific prediction instance. When the server
					 * processes the ability activation, it uses the same prediction key to reconcile (match up) the client's predicted
					 * action with the authoritative server execution. If the server's result matches the client's prediction, no
					 * correction is needed. If they differ (e.g., the ability failed on the server due to insufficient resources),
					 * the server sends a correction to the client using the prediction key to identify which predicted action to
					 * roll back or correct. This system allows for quick, responsive gameplay while maintaining server authority and preventing
					 * cheating or desync issues.
					 * 
					 * This allows the ability to respond to input being pressed while it's already running (e.g., charging attacks, 
					 * combo systems) by executing the InputPressed gameplay tag event handling logic defined within the ability 
					 * blueprint or C++ class.
					 * 
					 * THE InputPressed GAMEPLAY TAG EVENT SYSTEM:
					 * When this InvokeReplicatedEvent() call executes, it triggers any "Wait Gameplay Event" nodes inside the running
					 * ability blueprint that are listening for the "Ability.Event.InputPressed" gameplay tag. These event nodes act as
					 * listeners that pause execution until they receive their matching event tag, then continue the blueprint graph with
					 * the event data. This enables abilities to implement complex input-responsive behaviors like:
					 * 
					 * - Charging mechanics: A fireball ability could start charging when first activated, then release when input is
					 *   pressed again while the ability is still running (detected by this InputPressed event).
					 * - Combo systems: A melee attack ability could chain into different attacks based on timing additional input presses
					 *   during the attack animation (each press triggers this event, which the ability blueprint can respond to).
					 * - Hold-to-channel abilities: A healing ability could begin channeling on activation, then cancel or change behavior
					 *   when the player presses the input again (signaled by this event) while still channeling.
					 * - Multi-stage abilities: A teleport ability could mark a location on first activation, then teleport to that location
					 *   when the player presses the input a second time (the second press triggers this InputPressed event).
					 * 
					 * Without this event system, abilities could only respond to their initial activation. They would have no way to detect
					 * additional input presses while still running, severely limiting design possibilities. This event notification system
					 * transforms abilities from simple "fire-and-forget" actions into dynamic, interactive systems that can respond to
					 * player input throughout their entire execution lifecycle, enabling sophisticated ability designs commonly seen in
					 * action RPGs and MOBAs.
					 */
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
	}
}

void UFoxAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	// Check if the InputTag is not a valid tag that exists in this project. If so, return early.
	if (!InputTag.IsValid()) return;
	
	// Loop through the activatable abilities specs that this ASC has
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		/*
		 * Check if this ability spec's dynamic tags contain an exact match for the InputTag.
		 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer that holds this ability spec's dynamic tags
		 * (which includes the StartupInputTag we added in AddCharacterAbilities).
		 * HasTagExact() performs an exact match check. It returns true only if the InputTag exactly matches one of
		 * the tags in the container (not just a parent tag match).
		 */
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			/*
			 * Notify the ability spec that its associated input has been pressed.
			 * This is an internal bookkeeping function required by Unreal's ability system that marks the spec as
			 * having received input. This state is used by the ability system for various checks and may trigger
			 * ability-specific logic that responds to input press events.
			 */
			AbilitySpecInputPressed(AbilitySpec);

			/*
			 * Check if the ability is currently active (running).
			 * IsActive() returns false if the ability is not currently executing, meaning it's safe to activate it.
			 * Some abilities can be activated multiple times simultaneously (if configured), but this check ensures
			 * we only try to activate inactive abilities, preventing unwanted duplicate activations for abilities
			 * that should only run once at a time.
			 */
			if (!AbilitySpec.IsActive())
			{
				/*
				 * Attempt to activate the ability using its unique handle.
				 * TryActivateAbility() is the core GAS function that runs through all activation checks
				 * (cooldowns, costs, tags, etc.) and if all checks pass, executes the ability's ActivateAbility() function.
				 * The ability spec's Handle is a unique identifier for this specific ability instance.
				 * This function will fail gracefully if activation requirements aren't met, so it's safe to call.
				 */
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UFoxAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	// Check if the InputTag is not a valid tag that exists in this project. If so, return early.
	if (!InputTag.IsValid()) return;
	
	// Lock the ActivatableAbilities container until the for loop is done.
	FScopedAbilityListLock ScopedAbilityListLock(*this);
	
	// Loop through the activatable abilities specs that this ASC has
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		/*
		 * Check if this ability spec's dynamic tags contain an exact match for the InputTag.
		 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer that holds this ability spec's dynamic tags
		 * (which includes the StartupInputTag we added in AddCharacterAbilities).
		 * HasTagExact() performs an exact match check - it returns true only if the InputTag exactly matches one of
		 * the tags in the container (not just a parent tag match).
		 */
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			/*
			 * Notify the ability spec that its associated input has been released.
			 * This is an internal bookkeeping function required by Unreal's ability system that marks the specs input
			 * as having been released. This state is used by the ability system for various checks and may trigger
			 * ability-specific logic that responds to input released events.
			 */
			AbilitySpecInputReleased(AbilitySpec);
			
			/*
			 * Check if the ability is currently active (running).
			 * IsActive() returns false if the ability is not currently executing, meaning it's safe to activate it.
			 * Some abilities can be activated multiple times simultaneously (if configured), but this check ensures
			 * we only try to activate inactive abilities, preventing unwanted duplicate activations for abilities
			 * that should only run once at a time.
			 */
			if (AbilitySpec.IsActive())
			{
				/*
				 * Retrieve the primary instance of the ability for InstancedPerActor abilities.
				 * GetPrimaryInstance() returns a pointer to the main UGameplayAbility instance that was created when this
				 * ability was first activated. This function is only valid for abilities with the InstancedPerActor instancing
				 * policy, which creates one ability instance per actor that is reused across multiple activations. For abilities
				 * with other instancing policies (InstancedPerExecution, NonInstanced), this would return nullptr.
				 */
				UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance();
				
				/*
				 * Check if the primary instance pointer is valid before attempting to invoke the replicated event.
				 * This null check is necessary because GetPrimaryInstance() may return nullptr if called on an ability
				 * with an incompatible instancing policy, or if the ability instance hasn't been created yet due to timing
				 * or initialization issues. Without this check, attempting to call GetCurrentActivationInfo() on a null pointer
				 * would cause a crash. This safety guard ensures we only notify abilities that have valid instances.
				 */
				if (PrimaryInstance)
				{
					/* Do not Delete or simplify!!! This is profound and long for a reason.
					 * Notify the ability instance that its input has been released by invoking a replicated event.
					 * InvokeReplicatedEvent() is a GAS function that sends an ability event across the network in multiplayer games,
					 * ensuring both server and clients are synchronized about input state. We pass EAbilityGenericReplicatedEvent::InputReleased
					 * to indicate this is an input press event, AbilitySpec.Handle to identify which ability received the input,
					 * and the activation prediction key from GetCurrentActivationInfo().
					 * 
					 * The activation prediction key is critical for client-side prediction and server reconciliation. In networked
					 * gameplay, clients predict ability activations locally for responsive gameplay without waiting for server confirmation.
					 * Each predicted action gets a unique prediction key that identifies that specific prediction instance. When the server
					 * processes the ability activation, it uses the same prediction key to reconcile (match up) the client's predicted
					 * action with the authoritative server execution. If the server's result matches the client's prediction, no
					 * correction is needed. If they differ (e.g., the ability failed on the server due to insufficient resources),
					 * the server sends a correction to the client using the prediction key to identify which predicted action to
					 * roll back or correct. This system allows for quick, responsive gameplay while maintaining server authority and preventing
					 * cheating or desync issues.
					 * 
					 * This allows the ability to respond to input being released while it's already running (e.g., charging attacks, 
					 * combo systems) by executing the InputReleased gameplay tag event handling logic defined within the ability 
					 * blueprint or C++ class.
					 * 
					 * THE InputReleased GAMEPLAY TAG EVENT SYSTEM:
					 * When this InvokeReplicatedEvent() call executes, it triggers any "Wait Gameplay Event" nodes inside the running
					 * ability blueprint that are listening for the "Ability.Event.InputReleased" gameplay tag. These event nodes act as
					 * listeners that pause execution until they receive their matching event tag, then continue the blueprint graph with
					 * the event data. This enables abilities to implement complex input-responsive behaviors like:
					 * 
					 * - Charging mechanics: A fireball ability could start charging when first activated, then release when input is
					 *   released again while the ability is still running (detected by this InputReleased event).
					 * - Combo systems: A melee attack ability could chain into different attacks based on timing additional input presses
					 *   during the attack animation (each press triggers this event, which the ability blueprint can respond to).
					 * - Hold-to-channel abilities: A healing ability could begin channeling on activation, then cancel or change behavior
					 *   when the player presses the input again (signaled by this event) while still channeling.
					 * - Multi-stage abilities: A teleport ability could mark a location on first activation, then teleport to that location
					 *   when the player presses the input a second time (the second press triggers this InputReleased event).
					 * 
					 * Without this event system, abilities could only respond to their initial activation. They would have no way to detect
					 * additional input presses while still running, severely limiting design possibilities. This event notification system
					 * transforms abilities from simple "fire-and-forget" actions into dynamic, interactive systems that can respond to
					 * player input throughout their entire execution lifecycle, enabling sophisticated ability designs commonly seen in
					 * action RPGs and MOBAs.
					 */
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
	}
}

void UFoxAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	
	/*
	 * Create a scoped lock on this ASC's ability list to ensure thread-safe iteration.
	 * FScopedAbilityListLock prevents the ability list from being modified (abilities added/removed/changed)
	 * while we're iterating through it. This is critical in multiplayer games where abilities can be granted
	 * or revoked from other threads (e.g., server replication, gameplay effects). The lock is automatically
	 * released when ActiveScopeLock goes out of scope at the end of this function.
	 * We pass *this with the dereference operator to convert the 'this' pointer to a reference, since the constructor
	 * expects UAbilitySystemComponent& rather than a pointer.
	 */
	FScopedAbilityListLock ActiveScopeLock(*this);

	/*
	 * Iterate through all activatable ability specs granted to this ASC.
	 * GetActivatableAbilities() returns a TArray of FGameplayAbilitySpec representing all abilities
	 * this component can potentially activate. Each AbilitySpec contains the ability class, level,
	 * tags, and runtime state information. We use const reference to avoid unnecessary copying of
	 * the ability spec data during iteration.
	 */
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		
		/*
		 * Attempt to execute the Delegate input parameter's callback (the Lambda defined in 
		 * OverlayWidgetController.cpp) with the current ability spec, checking if execution succeeded.
		 * The callback extracts ability information (ability tag, input tag, status tag) from the AbilitySpec
		 * and broadcasts it through the AbilityInfo delegate to update UI elements with ability data.
		 * ExecuteIfBound() is a safe execution method that first checks if the delegate has a function bound to it.
		 * If bound, it calls that function passing the AbilitySpec as a parameter and returns true.
		 * If not bound (delegate is empty/null), it does nothing and returns false.
		 * The ! operator negates the result, so this if statement triggers when the delegate is NOT bound,
		 * indicating a programming error where ForEachAbility was called with an uninitialized/empty delegate.
		 */
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			/*
			 * Log an error message indicating delegate execution failed because no function was bound to it.
			 * This should never happen in normal execution - it indicates a bug where the caller passed an
			 * uninitialized delegate to ForEachAbility. LogFox is the custom log category for this project.
			 * %hs is a format specifier for a null-terminated string (the function name from __FUNCTION__).
			 * __FUNCTION__ is a compiler macro that expands to the current function's name at compile time,
			 * making the error message show exactly which function encountered this problem.
			 */
			UE_LOG(LogFox, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UFoxAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	
	/*
	 * Check if the AbilitySpec has a valid Ability instance pointer, which points to the actual ability associated with
	 * the spec. AbilitySpec.Ability is a TObjectPtr (weak object pointer) that may be null if the ability hasn't been
	 * instantiated yet or was garbage collected. We must verify it's valid before attempting to access its
	 * properties to avoid a null pointer dereference crash.
	 */
	if (AbilitySpec.Ability)
	{
		
		/*
		 * Iterate through all gameplay tags returned by the ability's GetAssetTags() method.
		 * GetAssetTags() returns a const reference to a FGameplayTagContainer that holds tags identifying what this
		 * ability is and does (e.g., "Abilities.Fire", "Abilities.Attack.Melee"). This method retrieves tags from
		 * the ability's class default object (CDO) rather than from the instance, making it efficient for querying
		 * static ability metadata. We use Get() to dereference the TObjectPtr and access the underlying UGameplayAbility
		 * object to call GetAssetTags(). Each Tag in the container is copied during iteration (not a reference),
		 * which is acceptable for lightweight FGameplayTag structs.
		*/
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			/*
			 * Check if this tag matches (is a child of or equal to) the "Abilities" parent tag.
			 * RequestGameplayTag() looks up a gameplay tag by name in the project's tag registry at runtime.
			 * FName("Abilities") converts the string literal to an FName for efficient tag lookup.
			 * MatchesTag() performs a hierarchical match - it returns true if Tag exactly matches "Abilities"
			 * or is a child tag (e.g., "Abilities.Fire" matches parent "Abilities", but "InputTag.LMB" does not).
			 */
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				/*
				 * Return the first tag found that matches the "Abilities" hierarchy.
				 * This tag uniquely identifies the ability type (e.g., "Abilities.Fire" or "Abilities.Jump")
				 * and can be used by other systems to query, display, or reference this specific ability.
				 * We return immediately upon finding the first match, and we assume each ability has only one
				 * primary identifying tag under the "Abilities" parent tag.
				 */
				return Tag;
			}
		}
	}
	/*
	 * Return an empty/invalid gameplay tag if no matching tag was found.
	 * FGameplayTag() constructs a default-initialized tag that is considered invalid/empty (IsValid() returns false).
	 * This serves as a safe fallback indicating that either the ability was null, had no tags, or had no tags
	 * matching the "Abilities" hierarchy. Callers should check IsValid() on the returned tag before using it.
	 */
	return FGameplayTag();
}

FGameplayTag UFoxAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	/*
	 * Iterate through all gameplay tags in the ability spec's dynamic tags container.
	 * GetDynamicSpecSourceTags() returns a FGameplayTagContainer that holds tags dynamically added to this ability spec
	 * at runtime (as opposed to tags defined on the ability class itself). In our system,
	 * this container holds the StartupInputTag that was added in AddCharacterAbilities(), which identifies
	 * which input action triggers this ability (e.g., "InputTag.LMB" or "InputTag.1"). Each Tag in the
	 * container is copied during iteration (not a reference), which is acceptable for lightweight FGameplayTag structs.
	 */
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		/*
		 * Check if this tag matches (is a child of or equal to) the "InputTag" parent tag.
		 * RequestGameplayTag() looks up a gameplay tag by name in the project's tag registry at runtime.
		 * FName("InputTag") converts the string literal to an FName for efficient tag lookup.
		 * MatchesTag() performs a hierarchical match - it returns true if Tag exactly matches "InputTag"
		 * or is a child tag (e.g., "InputTag.LMB" matches parent "InputTag", but "Abilities.Fire" does not).
		 */
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			/*
			 * Return the first tag found that matches the "InputTag" hierarchy.
			 * This tag identifies which input action is bound to this ability (e.g., "InputTag.LMB" or "InputTag.1")
			 * and is used by the input system to map player input to the correct ability activation.
			 * We return immediately upon finding the first match, and we assume each ability has only one
			 * input binding tag under the "InputTag" parent tag.
			 */
			return Tag;
		}
	}
	/*
	 * Return an empty/invalid gameplay tag if no matching tag was found.
	 * FGameplayTag() constructs a default-initialized tag that is considered invalid/empty (IsValid() returns false).
	 * This serves as a safe fallback indicating that either the ability spec had no dynamic tags, or had no tags
	 * matching the "InputTag" hierarchy. Callers should check IsValid() on the returned tag before using it.
	 */
	return FGameplayTag();
}

FGameplayTag UFoxAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	
	/*
	 * Iterate through all gameplay tags in the ability spec's dynamic tags container to find status tags.
	 * GetDynamicSpecSourceTags() returns a FGameplayTagContainer that holds tags dynamically added to this ability spec
	 * at runtime. In our system, this container holds status tags like "Abilities.Status.Equipped" or
	 * "Abilities.Status.Unlocked" that indicate the current state of the ability (whether it's equipped and ready to use,
	 * or unlocked but not equipped). Each StatusTag in the container is copied during iteration (not a reference),
	 * which is acceptable for lightweight FGameplayTag structs.
	 */
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		/*
		 * Check if this tag matches (is a child of or equal to) the "Abilities.Status" parent tag.
		 * RequestGameplayTag() looks up a gameplay tag by name in the project's tag registry at runtime.
		 * FName("Abilities.Status") converts the string literal to an FName for efficient tag lookup.
		 * MatchesTag() performs a hierarchical match. It returns true if StatusTag exactly matches "Abilities.Status"
		 * or is a child tag (e.g., "Abilities.Status.Equipped" matches parent "Abilities.Status", but "InputTag.LMB" does not).
		 */
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			/*
			 * Return the first tag found that matches the "Abilities.Status" hierarchy.
			 * This tag identifies the current status of the ability (e.g., "Abilities.Status.Equipped" or
			 * "Abilities.Status.Unlocked") and is used by UI systems to display the correct ability state,
			 * such as showing whether an ability is equipped and usable or needs to be equipped first.
			 * We return immediately upon finding the first match, and we assume each ability has only one
			 * status tag under the "Abilities.Status" parent tag at any given time.
			 */
			return StatusTag;
		}
	}
	/*
	 * Return an empty/invalid gameplay tag if no matching tag was found.
	 * FGameplayTag() constructs a default-initialized tag that is considered invalid/empty (IsValid() returns false).
	 * This serves as a safe fallback indicating that either the ability spec had no dynamic tags, or had no tags
	 * matching the "Abilities.Status" hierarchy. Callers should check IsValid() on the returned tag before using it.
	 */
	return FGameplayTag();
}

FGameplayTag UFoxAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	/*
	 * Attempt to retrieve the ability spec matching the AbilityTag and check if it exists (is not nullptr).
	 * GetSpecFromAbilityTag() searches through all activatable abilities for one with a matching ability tag and returns
	 * a const pointer to the FGameplayAbilitySpec if found, or nullptr if not found.
	 */
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		/*
		 * Extract and return the status tag from the found ability spec by calling GetStatusFromSpec().
		 * We dereference the Spec pointer with * to pass the actual FGameplayAbilitySpec object (not the pointer) to
		 * GetStatusFromSpec(), which searches through the spec's dynamic tags to find and return the status tag
		 * (e.g., "Abilities.Status.Equipped", "Abilities.Status.Unlocked"). This status tag indicates the ability's
		 * current state and is used by UI systems to display appropriate visual indicators showing whether the ability
		 * is ready to use, needs to be equipped, or is still locked.
		 */
		return GetStatusFromSpec(*Spec);
	}
	/*
	 * Return an empty/invalid gameplay tag if no ability spec was found matching the input AbilityTag.
	 * FGameplayTag() constructs a default-initialized tag that is considered invalid/empty (IsValid() returns false).
	 * This serves as a safe fallback indicating that the ability either hasn't been granted to this ASC yet, or the
	 * provided AbilityTag doesn't match any of the granted abilities. Callers should check IsValid() on the returned
	 * tag before using it to avoid treating the empty tag as a valid status.
	 */
	return FGameplayTag();
}

void UFoxAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	/*
	 * Attempt to retrieve the ability spec matching the AbilityTag and store it in a pointer variable for modification.
	 * GetSpecFromAbilityTag() searches through all activatable abilities for one with a matching ability tag and returns
	 * a pointer to the FGameplayAbilitySpec if found, or nullptr if not found. We use a non-const pointer because we need
	 * to modify the spec's dynamic tags (adding/removing slot and status tags) during the equip operation. The if statement
	 * checks if the pointer is not nullptr before proceeding, ensuring the ability exists and is granted to this ASC before
	 * attempting to equip it to the specified input slot.
	 */
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		/*
		 * Retrieve the gameplay tags singleton instance to access project-wide gameplay tags.
		 * FFoxGameplayTags::Get() returns a const reference to the singleton instance that holds all gameplay tags used
		 * in the project, providing efficient access to tags like Abilities_Status_Equipped and Abilities_Status_Unlocked
		 * without repeatedly looking them up in the tag registry. We store this as a const reference to avoid copying
		 * the entire tags object and to clearly indicate we're only reading tag values, not modifying them.
		 */
		const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

		/*
		 * Extract and store the ability's previous input slot tag (if any) before equipping it to the new slot.
		 * GetInputTagFromSpec() searches through the ability spec's dynamic tags to find and return the input tag
		 * (e.g., "InputTag.LMB", "InputTag.1") that identifies which input slot this ability was previously equipped to.
		 * We dereference the AbilitySpec pointer with * to pass the actual FGameplayAbilitySpec object to the function.
		 * This previous slot information is needed for the ClientEquipAbility RPC so UI systems can update both the old
		 * and new slot displays, removing the ability from its previous location and showing it in its new location.
		 */
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);

		/*
		 * Extract and store the ability's current status tag to verify it can be equipped.
		 * GetStatusFromSpec() searches through the ability spec's dynamic tags to find and return the status tag
		 * (e.g., "Abilities.Status.Equipped", "Abilities.Status.Unlocked") that indicates the ability's current state.
		 * We dereference the AbilitySpec pointer with * to pass the actual FGameplayAbilitySpec object to the function.
		 */
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		/*
		 * Validate that the ability's status allows it to be equipped by checking if it's either Equipped or Unlocked.
		 * This boolean check ensures the ability is in an appropriate state for equipping. An ability with
		 * Abilities_Status_Equipped can be moved to a different slot, and an ability with Abilities_Status_Unlocked
		 * can be equipped for the first time. Abilities with other statuses (like Eligible or Locked) cannot be equipped
		 * until they're purchased/unlocked. We use == for exact tag comparison rather than MatchesTag() because we need
		 * to check for these specific status values, not their parent or child tags. MatchesTagExact is used for
		 * strict equality check, and would likely work here, but this is the code the tutorial uses.
		 */
		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		
		// Checks if the variable created above is true or false
		if (bStatusValid)
		{
			/*
			 * Clear any ability currently occupying the target input slot before equipping the new ability.
			 * ClearAbilitiesOfSlot() iterates through all granted abilities and removes the Slot tag from any ability
			 * that currently has it in their dynamic tags. This ensures only one ability can be equipped to each input
			 * slot at a time. For example, if the player tries to equip a new ability to the LMB slot while another
			 * ability is already equipped there, this function will unequip the old ability first by removing its
			 * InputTag.LMB tag, freeing up the slot for the new ability.
			 */
			ClearAbilitiesOfSlot(Slot);

			/*
			 * Remove any previous input slot tag from the ability being equipped.
			 * ClearSlot() searches through the ability spec's dynamic tags for any tag matching the "InputTag" parent
			 * tag hierarchy and removes it. This is necessary when moving an ability from one input slot to another
			 * (e.g., moving an ability from the 1 key to the LMB slot). If we didn't clear the old slot tag first,
			 * the ability would keep both tags and respond to multiple different inputs simultaneously, which would
			 * cause incorrect behavior where pressing either the old or new input would activate the same ability.
			 */
			ClearSlot(AbilitySpec);

			/*
			 * Add the new input slot tag to the ability spec's dynamic tags container to bind it to the specified input.
			 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer where we store runtime tags for this ability spec.
			 * AddTag() adds the Slot parameter (e.g., "InputTag.LMB", "InputTag.1") to this container, creating the binding
			 * between this ability and the player's input action. After this line executes, when the player presses the
			 * corresponding input (handled in AFoxPlayerController::AbilityInputTagPressed/Held/Released), the input system
			 * will search for abilities with matching input tags and activate this ability. This completes the equip operation
			 * by establishing the new input-to-ability mapping.
			 */
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(Slot);

			/*
			 * Check if the ability's current status is exactly Unlocked to determine if we need to upgrade it to Equipped.
			 * MatchesTagExact() returns true only if Status exactly matches Abilities_Status_Unlocked (not just child or parent tags).
			 * This check handles the case where the player is equipping an ability they previously unlocked but never equipped.
			 * Abilities can exist in an Unlocked state where they've been purchased with spell points but aren't assigned to
			 * any input slot yet. When equipping such an ability for the first time, we need to transition its status from
			 * Unlocked to Equipped so UI systems can display it correctly and the ability becomes usable through player input.
			 * If the ability is already Equipped (being moved from one slot to another), this block won't execute since the
			 * status is already correct.
			 */
			if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				/*
				 * Remove the Abilities_Status_Unlocked tag from the ability spec's dynamic tags container.
				 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer where we store runtime status tags for this
				 * ability spec. RemoveTag() removes the Unlocked status tag because the ability is being equipped and needs
				 * to transition from "purchased but not equipped" to "equipped and ready to use". This tag removal ensures
				 * the ability no longer shows the unlocked status in UI systems, preparing it for the equipped status that
				 * will be added in the next line.
				 */
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Unlocked);

				/*
				 * Add the Abilities_Status_Equipped tag to the ability spec's dynamic tags container.
				 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer where we store runtime status tags for this
				 * ability spec. AddTag() marks this ability with the Equipped status, indicating the player has assigned it
				 * to an input slot (like LMB, RMB, or number keys) and can now activate it through player input. This status
				 * transition from Unlocked to Equipped makes the ability fully functional and usable in gameplay, and UI
				 * systems will update to show the ability as equipped with appropriate visual indicators.
				 */
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			/*
			 * Mark the ability spec as dirty to trigger replication to clients in multiplayer games.
			 * MarkAbilitySpecDirty() flags this specific ability spec as having changed, which tells the replication
			 * system to include it in the next network update sent to clients. This ensures that when playing in
			 * multiplayer (listen server or dedicated server), the equipped ability with its updated slot tag and status
			 * tag will replicate from the server to all clients, keeping the ability state synchronized across all
			 * connected players. This replication includes the new input slot binding and status we just modified,
			 * allowing clients to accurately display the ability's equipped state in their UI and activate it through
			 * the correct input. Without this call, clients wouldn't receive the updated ability spec until the next
			 * full ability array replication, causing desync between server and client ability states.
			 */
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		/*
		 * Call the client RPC to notify all clients that this ability has been equipped to a new slot.
		 * ClientEquipAbility() is a replicated function (marked with Client and _Implementation suffix) that executes
		 * on all clients, broadcasting the AbilityEquipped delegate with the ability's tag, equipped status, new slot,
		 * and previous slot. This ensures that when an ability is equipped on the server (in response to player input
		 * from the spell menu), the spell menu UI on all clients updates to reflect the change. We pass AbilityTag to
		 * identify which ability was equipped, Abilities_Status_Equipped to indicate the ability is now ready for use,
		 * Slot to show which input binding it was equipped to (e.g., "InputTag.LMB"), and PrevSlot to indicate which
		 * slot the ability was previously in (if any, or an empty tag if this was the first time equipping it). The
		 * client implementation will broadcast this information through the AbilityEquipped delegate, which the
		 * SpellMenuWidgetController and OverlayWidgetController listen to, allowing them to update the spell menu
		 * and spell globe UI elements to show the ability in its new slot and remove it from the old slot if applicable.
		 */
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UFoxAbilitySystemComponent::ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	/*
	 * Broadcast the AbilityEquipped multicast delegate to notify all registered listeners that an ability has been
	 * equipped to a new input slot.
	 * 
	 * This delegate broadcasts four key pieces of information about the equip operation:
	 * 1. AbilityTag: The gameplay tag identifying which specific ability was equipped (e.g., "Abilities.Fire",
	 *    "Abilities.Lightning"). This allows listeners to identify and display the correct ability information.
	 * 2. Status: The status tag for the ability after being equipped, which should always be "Abilities.Status.Equipped"
	 *    since this RPC is only called after successfully equipping an ability. This confirms the ability is ready to use.
	 * 3. Slot: The new input slot tag where the ability was equipped (e.g., "InputTag.LMB", "InputTag.1"). This tells
	 *    listeners which input slot now activates this ability, allowing UI systems to display the ability in the correct
	 *    spell globe or ability bar position.
	 * 4. PreviousSlot: The input slot tag where the ability was previously equipped (if any), or an empty tag if this
	 *    is the first time equipping the ability. This allows UI systems to clear the ability from its old location
	 *    when moving it to a new slot, ensuring the UI accurately reflects that the ability is no longer bound to the
	 *    previous input.
	 * 
	 * Listeners (such as SpellMenuWidgetController and OverlayWidgetController) use this broadcast to update their
	 * spell menu and spell globe displays, moving the ability's icon and information from the previous slot to the new
	 * slot, and updating visual indicators showing which abilities are equipped and ready to use. This ensures the UI
	 * stays synchronized with the actual input-to-ability mappings maintained by the ability system component.
	 */
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

FGameplayTag UFoxAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	/*
	 * Attempt to retrieve the ability spec matching the AbilityTag and check if it exists (is not nullptr).
	 * GetSpecFromAbilityTag() searches through all activatable abilities for one with a matching ability tag and returns
	 * a const pointer to the FGameplayAbilitySpec if found, or nullptr if not found.
	 */
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		/*
		 * Extract and return the input tag from the found ability spec by calling GetInputTagFromSpec().
		 * We dereference the Spec pointer with * to pass the actual FGameplayAbilitySpec object (not the pointer) to
		 * GetInputTagFromSpec(), which searches through the spec's dynamic tags to find and return the input tag
		 * (e.g., "InputTag.LMB", "InputTag.1") that identifies which input action triggers this ability. This input tag
		 * is used by the input system to map player input to the correct ability activation, allowing the controller
		 * to activate this ability when the player presses the corresponding input key or button.
		 */
		return GetInputTagFromSpec(*Spec);
	}
	/*
	 * Return an empty/invalid gameplay tag if no ability spec was found matching the input AbilityTag parameter.
	 * FGameplayTag() constructs a default-initialized tag that is considered invalid/empty (IsValid() returns false).
	 * This serves as a safe fallback indicating that the ability either hasn't been granted to this ASC yet, or the
	 * provided AbilityTag doesn't match any of the granted abilities. Callers should check IsValid() on the returned
	 * tag before using it to avoid treating the empty tag as a valid input binding.
	 */
	return FGameplayTag();
}

FGameplayAbilitySpec* UFoxAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
		
	/*
	 * Create a scoped lock on this ASC's ability list to ensure thread-safe iteration.
	 * FScopedAbilityListLock prevents the ability list from being modified (abilities added/removed/changed)
	 * while we're iterating through it. This is critical in multiplayer games where abilities can be granted
	 * or revoked from other threads (e.g., server replication, gameplay effects). The lock is automatically
	 * released when ActiveScopeLoc goes out of scope at the end of this function.
	 * We pass *this with the dereference operator to convert the 'this' pointer to a reference, since the constructor
	 * expects UAbilitySystemComponent& rather than a pointer. 
	 * 
	 * The dereference operator (*) on a pointer doesn't return
	 * a "regular value" (which would be a copy). Instead, it returns a value reference to the object the pointer
	 * points to. This reference can then be used for the reference input parameter (UAbilitySystemComponent&) of the
	 * ActiveScopeLock() function.
	 * 
	 * In C++, when you dereference a pointer with *, you get a reference to the pointed-to object, not a copy.
	 * So *this produces a reference to the current UFoxAbilitySystemComponent object, which is exactly what the
	 * FScopedAbilityListLock constructor needs. If we passed 'this' directly (the pointer), it would be a type
	 * mismatch (pointer vs reference). This function takes a reference, because the lock needs to lock the actual 
	 * ability list, not a copy of it.
	 */
	FScopedAbilityListLock ActiveScopeLoc(*this);

	/*
	 * Iterate through all activatable ability specs granted to this ASC.
	 * GetActivatableAbilities() returns a TArray of FGameplayAbilitySpec representing all abilities
	 * this component can potentially activate. We use a non-const reference (FGameplayAbilitySpec&) because
	 * we need to return a pointer to the spec if found, and returning a pointer to a const spec would lose
	 * the ability to modify it through the returned pointer. Each AbilitySpec contains the ability class, level,
	 * tags, and runtime state information.
	 */
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		
		/*
		 * Iterate through all ability tags defined on this ability's class to check for a match with the input parameter.
		 * 
		 * Breaking down the expression AbilitySpec.Ability.Get()->GetAssetTags():
		 * 
		 * 1. AbilitySpec.Ability: This accesses the Ability member of FGameplayAbilitySpec, which is a TObjectPtr<UGameplayAbility>.
		 *    The TObjectPtr holds a reference to the actual UGameplayAbility instance associated with this spec. Think 
		 *    of UGameplayAbility as a C++ template or recipe for an ability. You create a Blueprint in the Unreal Editor
		 *    that inherits from UGameplayAbility (like creating a cake recipe from a base recipe template). That Blueprint
		 *    defines what the ability does, what it costs, its cooldown, etc. When the game runs, that Blueprint becomes
		 *    a real object in memory. `AbilitySpec.Ability` points to that object. So UGameplayAbility is the C++ base class,
		 *    your Blueprint is the specific ability design you made in the editor, and AbilitySpec.Ability is the actual
		 *    running instance of that ability in the game.
		 * 
		 * 2. .Get(): This method call dereferences the TObjectPtr and returns a raw pointer (UGameplayAbility*) to the underlying
		 *    UGameplayAbility object. We must call Get() because TObjectPtr doesn't directly expose the underlying object's members.
		 *    It must be explicitly converted to a raw pointer first.
		 * 
		 * 3. GetAssetTags(): This method returns a const reference to a FGameplayTagContainer that holds gameplay tags identifying
		 *    what this ability is and does (e.g., "Abilities.Fire", "Abilities.Attack.Melee"). GetAssetTags() retrieves tags from
		 *    the ability's class default object (CDO) rather than from instance data, making it efficient for querying static
		 *    ability metadata. These tags are typically populated in the ability's blueprint under the "Asset Tags" section.
		 * 
		 * Each Tag in the container is copied during iteration (not a reference), which is acceptable for
		 * lightweight FGameplayTag structs.
		 */
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->GetAssetTags())
		{
			/*
			 * Check if this ability tag matches (is equal to or a child of) the input AbilityTag parameter.
			 * MatchesTag() performs a hierarchical match. It returns true if Tag exactly matches AbilityTag
			 * or if Tag is a child of AbilityTag (e.g., "Abilities.Fire.Firebolt" matches "Abilities.Fire").
			 * This allows flexible querying where you can search for specific abilities or groups of abilities
			 * by specifying a parent tag.
			 */
			if (Tag.MatchesTag(AbilityTag))
			{
				/*
				 * Return a pointer to the matching ability spec immediately upon finding the first match.
				 * We use the address-of operator (&) to convert the AbilitySpec reference to a pointer, which
				 * allows the caller to modify the spec if needed (e.g., adding/removing tags, changing level).
				 * This assumes each ability has a unique identifying tag and returns the first match found,
				 * exiting both loops early for efficiency.
				 */
				return &AbilitySpec;
			}
		}
	}
	/*
	 * Return nullptr if no ability spec was found with a tag matching the input AbilityTag parameter.
	 * This serves as a safe fallback indicating that either this ASC has no abilities granted, or none of
	 * the granted abilities have tags matching the requested tag. Callers should check for nullptr before
	 * attempting to dereference the returned pointer to avoid crashes.
	 */
	return nullptr;
}

void UFoxAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	/*
	 * Check if the avatar actor (the character/pawn owning this ASC) implements the UPlayerInterface.
	 * GetAvatarActor() returns the AActor that this ability system component is attached to and representing.
	 * Implements<UPlayerInterface>() is a template function that checks if the actor's class implements the specified
	 * interface. UPlayerInterface is a Blueprint-accessible interface that exposes player-specific
	 * functionality like attribute points management. This check is necessary because not all actors with ASCs
	 * are player characters (enemies, NPCs, etc. won't have attribute points), so we must verify the interface
	 * is implemented before attempting to call its functions to avoid runtime errors.
	 */
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		/*
		 * Verify that the player has at least one available attribute point to spend before proceeding with the upgrade.
		 * Execute_GetAttributePoints() is the interface function execution macro that safely calls the GetAttributePoints
		 * Blueprint Native Event defined in IPlayerInterface, returning the current number of unspent attribute points
		 * the player has available. We pass GetAvatarActor() as the target object to query. The > 0 comparison ensures
		 * the player has a positive number of points - if they have 0 or negative points (which shouldn't happen but
		 * we guard against it), the upgrade request is silently rejected client-side before even sending an RPC to the server,
		 * preventing invalid upgrade attempts.
		 */
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			/*
			 * Call the server RPC to perform the attribute upgrade with the specified attribute tag on the authoritative server.
			 * ServerUpgradeAttribute() is a replicated function (marked with Server and _Implementation suffix) that executes
			 * on the server regardless of where this function was called from (client or server). The AttributeTag parameter
			 * identifies which attribute to upgrade (e.g., "Attributes.Vital.Strength" or "Attributes.Vital.Intelligence")
			 * and is passed through to the server for validation and processing. The server implementation will send a
			 * gameplay event with this tag to activate the appropriate attribute upgrade ability, and will deduct one
			 * attribute point from the player's available points. Using an RPC ensures the server has authority over
			 * attribute upgrades, preventing client-side cheating where a player could modify their attributes without
			 * spending points or spending more points than they have.
			 */
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UFoxAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	/*
	 * Create a FGameplayEventData struct to hold payload information for the gameplay event we're about to send.
	 * FGameplayEventData is the standard container for passing contextual information when triggering gameplay events
	 * in the ability system. This payload will carry data about which attribute is being upgraded and by how much,
	 * allowing the receiving ability to process the upgrade request with all necessary information.
	 */
	FGameplayEventData Payload;

	/*
	 * Set the EventTag of the payload to the AttributeTag parameter identifying which attribute to upgrade.
	 * The EventTag serves as the primary identifier for this event and will be matched against ability trigger tags or
	 * Event Tags on Wait Gameplay Event blueprint nodes to determine which ability should respond to this event.
	 * For example, if AttributeTag is "Attributes.Vital.Strength", the ability system will activate any ability 
	 * listening for that specific tag (our GA_ListenForEvent ability is configured to trigger when they receive an 
	 * event with their matching attribute tag).
	 */
	Payload.EventTag = AttributeTag;

	/*
	 * Set the EventMagnitude to 1.f to indicate we want to upgrade the attribute by one level.
	 * EventMagnitude is a float value that can carry numerical data with the event payload. In our attribute upgrade
	 * system, this value represents how many levels to upgrade the attribute (always 1 in our current implementation
	 * since we upgrade one level at a time per attribute point spent). The receiving ability can read this magnitude
	 * value and use it to determine how much to increase the attribute.
	 */
	Payload.EventMagnitude = 1.f;

	/*
	 * Send a gameplay event to the avatar actor with the AttributeTag and Payload to trigger the attribute upgrade 
	 * ability (GA_ListenForEvent).
	 * 
	 * SendGameplayEventToActor() is a GAS helper function that broadcasts a gameplay event to the specified actor's ASC,
	 * causing any abilities with matching event triggers or Event Tags on Wait Gameplay Event blueprint nodes to 
	 * activate. We pass GetAvatarActor() as the target (the player
	 * character whose attribute we're upgrading), AttributeTag as the event tag to match against ability triggers,
	 * and Payload containing the upgrade data. This will activate the corresponding attribute upgrade ability
	 * (e.g., GA_ListenForEvent) which applies a gameplay effect to permanently increase the attribute.
	 */
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	/*
	 * Check if the avatar actor (the owner of this ASC) implements the UPlayerInterface to verify it has attribute point
	 * management functionality. This check is necessary because only player characters should have attribute points to
	 * spend, while other actors with ASCs (enemies, NPCs) should not. We verify the interface is implemented before 
	 * attempting to deduct points to prevent crashes or errors when calling interface functions on actors that don't 
	 * implement UPlayerInterface.
	 */
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		/*
		 * Deduct one attribute point from the player's available attribute points by calling AddToAttributePoints with -1.
		 * Execute_AddToAttributePoints() is the interface function execution macro that safely calls the AddToAttributePoints
		 * Blueprint Native Event defined in IPlayerInterface, modifying the player's unspent attribute point count.
		 * We pass GetAvatarActor() as the target and -1 as the value to subtract (adding a negative value effectively
		 * subtracts). This deduction happens after successfully sending the upgrade event, ensuring the player is charged
		 * the attribute point cost for upgrading their attribute. The function will update the player's attribute point
		 * count and broadcast any necessary delegates to update UI displays showing remaining attribute points.
		 */
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UFoxAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	
	/*
	 * Retrieve the AbilityInfo data asset that contains information about all abilities in the game.
	 * GetAbilityInfo() is a static utility function from UFoxAbilitySystemLibrary that looks up the project's
	 * ability information data asset, which stores metadata for all abilities including their tags, level requirements,
	 * ability classes, descriptions, and icons. We pass GetAvatarActor() as the world context object so the function will 
	 * get the AbilityInfo from the world in which that actor exists.
	 * This data asset serves as the master reference for ability information and is used to determine which abilities
	 * should become available to the player as they level up.
	 */
	UAbilityInfo* AbilityInfo = UFoxAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	/*
	 * Iterate through all ability information entries in the AbilityInformation array from the data asset.
	 * AbilityInformation is a TArray<FFoxAbilityInfo> containing metadata for every ability in the game.
	 * Each FFoxAbilityInfo struct holds data like the ability's tag, level requirement, ability class reference,
	 * description text, and icon texture. We iterate through this array to check which abilities should be
	 * marked as eligible (available for purchase/unlock) based on the player's current level.
	 * We use a const reference to avoid copying the struct data during iteration.
	 */
	for (const FFoxAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		/*
		 * Skip this ability if it doesn't have a valid ability tag.
		 * IsValid() checks if the AbilityTag is a properly initialized gameplay tag that exists in the project's
		 * tag registry. An invalid tag indicates a configuration error in the data asset where an ability entry
		 * was not properly set up with an identifying tag. We use continue to skip to the next iteration rather
		 * than processing an ability that can't be properly identified or queried by other systems.
		 */
		if (!Info.AbilityTag.IsValid()) continue;

		/*
		 * Skip this ability if the player's current level is below the ability's level requirement.
		 * LevelRequirement is an integer field in FFoxAbilityInfo that specifies the minimum character level
		 * needed to unlock this ability. This comparison ensures abilities are only marked as eligible when
		 * the player has reached the appropriate level, implementing the progression system where higher-level
		 * abilities become available as the player levels up. We use continue to skip processing this ability
		 * until the player reaches the required level in a future level-up.
		 */
		if (Level < Info.LevelRequirement) continue;
		
		
		/*
		 * Check if this ability has not yet been granted to this ASC by attempting to find its ability spec.
		 * GetSpecFromAbilityTag() searches through all activatable abilities for one matching the Info.AbilityTag
		 * and returns a pointer to the matching FGameplayAbilitySpec if found, or nullptr if not found.
		 * If the ability doesn't exist (nullptr), we need to create and grant it to this ASC so it becomes
		 * available for the player to purchase/unlock from the spell menu. This check ensures we only grant
		 * each ability once, preventing duplicate ability specs that would waste memory and cause UI issues.
		 */
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			/*
			 * Create a new ability spec for this ability at level 1 using the FGameplayAbilitySpec constructor.
			 * FGameplayAbilitySpec is the container that holds all runtime information about an ability instance,
			 * including the class if the ability (Info.Ability is the TSubclassOf<UGameplayAbility> from the data asset), level,
			 * dynamic tags, and activation state. We initialize it at level 1 because the player hasn't purchased
			 * or learned this ability yet. It's just becoming available as an option. The level will be increased
			 * later when the player actually unlocks and upgrades the ability through the spell menu system.
			 */
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);

			/*
			 * Mark this ability as eligible for purchase by adding the Abilities_Status_Eligible tag to its dynamic tags.
			 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer for this spec where we store runtime status tags.
			 * FFoxGameplayTags::Get().Abilities_Status_Eligible is the gameplay tag that identifies abilities the player
			 * can see and purchase in the spell menu UI but hasn't unlocked yet. This status is distinct from equipped
			 * abilities (which the player can use) or locked abilities (which the player can't yet see). The UI will query
			 * this tag to display the ability in the spell menu with a visual indication that it's available for purchase
			 * at the player's current level.
			 */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FFoxGameplayTags::Get().Abilities_Status_Eligible);

			/*
			 * Grant the newly created ability spec to this ASC, adding it to the list of activatable abilities.
			 * GiveAbility() is the core GAS function that takes ownership of the ability spec and adds it to the
			 * ActivatableAbilities array managed by this component. Then, it can be queried, displayed in UI, and 
			 * eventually activated once the player purchases it. The ability is not yet usable (it's marked eligible,
			 * not equipped), but granting it now ensures it appears in the spell menu for the player to see and purchase
			 * when they're ready.
			 */
			GiveAbility(AbilitySpec);

			/*
			 * Mark the ability spec as dirty to trigger replication to clients in multiplayer games.
			 * MarkAbilitySpecDirty() flags this specific ability spec as having changed, which tells the replication
			 * system to include it in the next network update sent to clients. This ensures that when playing in
			 * multiplayer (listen server or dedicated server), the newly granted eligible ability will replicate from
			 * the server to all clients, allowing the spell menu UI to display consistently across all connected players.
			 * Without this call, clients wouldn't receive the updated ability list until the next full ability array
			 * replication, causing desync between what the server knows and what clients see in their spell menus.
			 */
			MarkAbilitySpecDirty(AbilitySpec);
			
			/*
			 * Call the client RPC to notify all clients that this ability's status has changed to eligible.
			 * ClientUpdateAbilityStatus() is a replicated function (marked with Client and _Implementation suffix) that
			 * executes on all clients, broadcasting the AbilityStatusChanged delegate with the ability's tag, new status tag,
			 * and ability level. This ensures that when an ability becomes eligible due to the player leveling up, the spell
			 * menu UI on all clients updates to display the newly available ability with its correct level. We pass
			 * Info.AbilityTag to identify which ability changed, Abilities_Status_Eligible to indicate the ability is now
			 * available for purchase but not yet equipped, and 1 as the initial ability level since newly eligible abilities
			 * start at level 1. The client implementation will broadcast this information through the AbilityStatusChanged
			 * delegate, which the SpellMenuWidgetController listens to, allowing it to update the UI with the new ability
			 * status, level, and display the ability in the spell menu with appropriate visual indicators showing it can
			 * now be purchased.
			 */
			ClientUpdateAbilityStatus(Info.AbilityTag, FFoxGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UFoxAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	/*
	 * Attempt to retrieve the ability spec matching the AbilityTag and check if it exists (is not nullptr).
	 * GetSpecFromAbilityTag() searches through all activatable abilities for one with a matching ability tag and returns
	 * a pointer to the FGameplayAbilitySpec if found, or nullptr if not found.
	 */
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		/*
		 * Check if the avatar actor (the owner of this ASC) implements the UPlayerInterface to verify it has spell point
		 * management functionality. This check is necessary because only player characters should have spell points to
		 * spend, while other actors with ASCs (enemies, NPCs) should not. We verify the interface is implemented before
		 * attempting to deduct points to prevent crashes or errors when calling interface functions on actors that don't
		 * implement UPlayerInterface. This check acts as a safety guard, ensuring the spell point system is only applied
		 * to appropriate player controlled characters.
		 */
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			/*
			 * Deduct one spell point from the player's available spell points by calling AddToSpellPoints with -1.
			 * Execute_AddToSpellPoints() is the interface function execution macro that safely calls the AddToSpellPoints
			 * Blueprint Native Event defined in IPlayerInterface, modifying the player's unspent spell point count.
			 * We pass GetAvatarActor() as the target and -1 as the value to subtract (adding a negative value effectively
			 * subtracts). This deduction happens before modifying the ability's status or level, ensuring the player is
			 * charged the spell point cost regardless of what type of upgrade they're purchasing (unlocking a new ability
			 * or leveling up an existing one). The function will update the player's spell point count and broadcast any
			 * necessary delegates to update UI displays showing remaining spell points.
			 */
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		/*
		 * Retrieve the gameplay tags singleton instance to access project wide gameplay tags.
		 * FFoxGameplayTags::Get() returns a const reference to the singleton instance that holds all gameplay tags used
		 * in the project, providing efficient access to tags like Abilities_Status_Eligible, Abilities_Status_Unlocked,
		 * and Abilities_Status_Equipped without repeatedly looking them up in the tag registry.
		 */
		const FFoxGameplayTags GameplayTags = FFoxGameplayTags::Get();

		/*
		 * Retrieve the current status tag of the ability being upgraded.
		 * GetStatusFromSpec() searches through the ability spec's dynamic tags container to find and return the status
		 * tag (e.g., "Abilities.Status.Eligible", "Abilities.Status.Unlocked") that indicates the ability's current
		 * state.
		 */
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		
		/*
		 * Check if the ability's current status is exactly "Abilities.Status.Eligible" to determine if this is an
		 * initial ability unlock. MatchesTagExact() returns true only if Status exactly matches Abilities_Status_Eligible,
		 * not if it's a child or parent tag. 
		 */
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			/*
			 * Remove the Abilities_Status_Eligible tag from the ability spec's dynamic tags container.
			 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer where we store runtime status tags for this
			 * ability spec. RemoveTag() removes the Eligible status tag since the player has now spent a spell point to
			 * unlock this ability, transitioning it from "available for purchase" to "purchased and unlocked".
			 * This tag removal ensures the ability no longer appears as eligible in the spell menu UI.
			 */
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);

			/*
			 * Add the Abilities_Status_Unlocked tag to the ability spec's dynamic tags container. GetDynamicSpecSourceTags() 
			 * returns the FGameplayTagContainer where we store runtime status tags for this ability spec.
			 * AddTag() marks this ability with the Unlocked status, indicating the player has purchased it but hasn't
			 * yet equipped it to any input slot (like LMB, RMB, or number keys). Unlocked abilities appear in the
			 * spell menu with visual indicators showing they're owned but not equipped, and can be equipped to any
			 * available input slot through the equip button. This status is distinct from Equipped (ready to use)
			 * and Eligible (not yet purchased)
			 */
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);

			/*
			 * Update the local Status variable to reflect the ability's new Unlocked status.
			 * This variable update is necessary because we use Status in the ClientUpdateAbilityStatus() RPC call
			 * below to notify clients of the status change. Without updating this variable, the RPC would send the
			 * old Eligible status instead of the new Unlocked status.
			 */
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		/*
		 * Check if the ability's current status is either Equipped or Unlocked to determine if this is a level-up
		 * operation. MatchesTagExact() returns true only if Status exactly matches either Abilities_Status_Equipped
		 * or Abilities_Status_Unlocked (not if it matches child or parent tags). If the ability is already unlocked or equipped,
		 * the player is spending a spell point to increase the ability's level rather than unlocking it for the
		 * first time. This allows players to invest additional spell points into abilities they already own to make
		 * them more powerful
		 */
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			/*
			 * Increment the ability's level by 1 to reflect the upgrade from spending a spell point.
			 * AbilitySpec->Level is an integer representing the ability's current power level, starting at 1 when
			 * first unlocked. Increasing this value makes the ability stronger (more damage, lower cooldown, etc.)
			 * based on how the ability's gameplay effects scale with the level in their configurations.
			 */
			AbilitySpec->Level += 1;
		}
		
		/*
		 * Call the client RPC to notify all clients that this ability's status or level has changed.
		 * ClientUpdateAbilityStatus() is a replicated function (marked with Client and _Implementation suffix) that
		 * executes on all clients, broadcasting the AbilityStatusChanged delegate with the ability's tag, updated status tag,
		 * and new level. This ensures that when an ability is unlocked or leveled up on the server (due to spending a spell point),
		 * the spell menu UI on all clients updates to reflect the change. We pass AbilityTag to identify which ability changed,
		 * Status to indicate the ability's current state (Unlocked if just purchased, or Equipped/Unlocked if leveled up),
		 * and AbilitySpec->Level to show the ability's new power level. This function will broadcast this information
		 * through the AbilityStatusChanged delegate, which the SpellMenuWidgetController listens to, allowing it to update the UI
		 * with the new ability status and level, refreshing spell globe displays and button states accordingly.
		 */
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);

		/*
		 * Mark the ability spec as dirty to trigger replication to clients in multiplayer games.
		 * MarkAbilitySpecDirty() flags this specific ability spec as having changed, which tells the replication
		 * system to include it in the next network update sent to clients. This ensures that when playing in
		 * multiplayer (listen server or dedicated server), the modified ability (whether newly unlocked or leveled up)
		 * will replicate from the server to all clients, keeping the ability state synchronized across all connected players.
		 * This replication includes the updated level and status tags we just modified, allowing clients to accurately
		 * display the ability's current state in their spell menu UI. Without this call, clients wouldn't receive the
		 * updated ability spec until the next full ability array replication, causing desync between what the server
		 * knows and what clients see in their spell menus.
		 */
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

bool UFoxAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
	FString& OutNextLevelDescription)
{
	/*
	 * Attempt to retrieve the ability spec matching the AbilityTag and check if it exists (is not nullptr).
	 * GetSpecFromAbilityTag() searches through all activatable abilities for one with a matching ability tag and returns
	 * a const pointer to the FGameplayAbilitySpec if found, or nullptr if not found. If the ability spec exists,
	 * it means the player has this ability granted (either unlocked, equipped, or eligible), so we can retrieve
	 * its descriptions from the ability class instance. If nullptr, the ability is locked and hasn't been granted yet,
	 * requiring us to display the locked description instead (handled in the else block below).
	 */
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		/*
		 * Attempt to cast the ability from the spec to UFoxGameplayAbility and check if the cast succeeds.
		 * AbilitySpec->Ability is a TObjectPtr<UGameplayAbility> pointing to the actual ability instance, which we need
		 * to cast to our custom UFoxGameplayAbility subclass to access its GetDescription and GetNextLevelDescription
		 * functions. Cast<UFoxGameplayAbility>() performs a safe dynamic cast and returns a valid pointer if the ability
		 * is an instance of UFoxGameplayAbility (or one of its subclasses), or nullptr if the cast fails. This cast
		 * should always succeed in our project since all our abilities inherit from UFoxGameplayAbility, but we check
		 * anyway as a safety measure to prevent crashes if a non-Fox ability is accidentally granted to this ASC.
		 */
		if(UFoxGameplayAbility* AuraAbility = Cast<UFoxGameplayAbility>(AbilitySpec->Ability))
		{
			/*
			 * Retrieve the ability's description text for its current level and assign it to the output parameter.
			 * GetDescription() is a virtual function defined in FoxGameplayAbility.cpp that returns a formatted FString
			 * containing the ability's description.
			 * 
			 * We pass AbilitySpec->Level to get the description tailored to the ability's current power level, which
			 * may include level-specific damage values, cooldowns, or other scaling information. The result is assigned
			 * to OutDescription, an output parameter passed by reference, allowing the caller to receive the description
			 * string and display it in the spell menu UI.
			 */
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);

			/*
			 * Retrieve the ability's next level description text and assign it to the output parameter.
			 * GetNextLevelDescription() is a virtual function defined in FoxGameplayAbility.cpp that returns a formatted
			 * FString describing what benefits the player will gain if they upgrade the ability to the next level.
			 * We pass AbilitySpec->Level + 1 to get the description for the next power level, showing the player what
			 * improvements they can expect (increased damage, reduced cooldown, etc.) if they spend a spell point to
			 * upgrade. The result is assigned to OutNextLevelDescription, an output parameter passed by reference,
			 * allowing the caller to receive the next level description and display it in the spell menu UI below
			 * the current description.
			 */
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);

			/*
			 * Return true to indicate that we successfully retrieved both descriptions for an unlocked/granted ability.
			 * This boolean return value signals to the caller that the ability is available (unlocked, equipped, or
			 * eligible) and that both OutDescription and OutNextLevelDescription have been populated with valid
			 * description strings from the ability instance.
			 */
			return true;
		}
	}
	/*
	 * Retrieve the AbilityInfo data asset containing metadata for all abilities in the game to access locked ability info.
	 * UFoxAbilitySystemLibrary::GetAbilityInfo() is a static utility function that looks up the project's ability
	 * information data asset, which stores metadata like ability tags, level requirements, descriptions, and icons.
	 * We reach this code path when the ability spec wasn't found (ability is locked/not granted yet) or when the cast
	 * to UFoxGameplayAbility failed. We pass GetAvatarActor() as the world context object to retrieve the AbilityInfo
	 * from the correct world. This data asset is needed to look up the level requirement for the locked ability so we
	 * can display a message telling the player what level they need to reach to unlock this ability.
	 */
	const UAbilityInfo* AbilityInfo = UFoxAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	/*
	 * Check if the AbilityTag is invalid or matches the special "Abilities.None" tag that represents no ability.
	 * !AbilityTag.IsValid() returns true if the tag is not properly initialized or doesn't exist in the tag registry,
	 * indicating a programming error or uninitialized state. AbilityTag.MatchesTagExact() returns true if the tag
	 * exactly matches FFoxGameplayTags::Get().Abilities_None, which is a special tag used to represent empty ability
	 * slots in the UI (like empty spell globes that haven't had an ability equipped yet). If either condition is true,
	 * we should not attempt to look up ability information from the data asset because there's no valid ability to
	 * display, and we should instead clear the description to show nothing in the UI.
	 */
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FFoxGameplayTags::Get().Abilities_None))
	{
		/*
		 * Assign an empty string to the description output parameter when the ability tag is invalid or represents no ability.
		 * FString() constructs a default empty string. This clears any previous description text and signals to the spell
		 * menu UI that there's no ability to display for this slot, so it should nothing instead
		 * of attempting to display ability information. This handles edge cases like uninitialized ability slots or the
		 * special "None" ability tag used for empty spell globe slots in the UI.
		 */
		OutDescription = FString();
	}
	else
	{
		/*
		 * Generate and assign a locked ability description string to the output parameter, showing the level requirement.
		 * UFoxGameplayAbility::GetLockedDescription() is a static helper function that returns a formatted FString
		 * indicating the ability is locked and showing what level is required to unlock it. We pass
		 * AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement, which looks up the ability's metadata struct from
		 * the data asset by its tag and retrieves the LevelRequirement field specifying the minimum character level needed to unlock
		 * this ability. The resulting string (e.g., "Spell Locked Until Level: 5") is assigned to OutDescription so the
		 * spell menu UI can display it to inform the player when this ability will become available.
		 */
		OutDescription = UFoxGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}

	/*
	 * Assign an empty string to the next level description output parameter since locked abilities should not show a 
	 * next level description.
	 * FString() constructs a default empty string. For locked abilities that the player hasn't unlocked yet, there's
	 * no meaningful "next level" description to show since the ability isn't available at all. The spell menu UI will
	 * use this empty string to hide or leave blank the next level description section, showing only the locked message
	 * in the main description area.
	 */
	OutNextLevelDescription = FString();

	/*
	 * Return false to indicate that the ability is locked and we provided a locked description instead of real ability
	 * descriptions. This boolean return value signals to the caller that the ability is not yet available to the player
	 * (hasn't reached the required level to unlock it) and that OutDescription contains a locked message rather than
	 * the ability's actual description.
	 */
	return false;
}

void UFoxAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	/*
	 * Extract the current input slot tag from the ability spec to identify which input binding needs to be cleared.
	 * GetInputTagFromSpec() searches through the spec's dynamic tags to find and return the input tag
	 * (e.g., "InputTag.LMB", "InputTag.1") that identifies which input slot this ability is currently equipped to.
	 * We dereference the Spec pointer with * to pass the actual FGameplayAbilitySpec object to the function.
	 */
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);

	/*
	 * Remove the input slot tag from the ability spec's dynamic tags container to clear the input binding.
	 * GetDynamicSpecSourceTags() returns the FGameplayTagContainer where we store runtime tags for this ability spec.
	 * RemoveTag() removes the Slot tag (the input tag we just extracted) from this container, breaking the connection
	 * between the ability and its input slot. After this removal, the ability is no longer bound to any input action
	 * and cannot be activated through player input until it's re-equipped to a new slot. This operation is typically
	 * performed when moving an ability to a different slot or when clearing a slot to make room for a different ability.
	 */
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);

	/*
	 * Mark the ability spec as dirty to trigger replication to clients in multiplayer games.
	 * MarkAbilitySpecDirty() flags this specific ability spec as having changed, which tells the replication
	 * system to include it in the next network update sent to clients. This ensures that when playing in
	 * multiplayer (listen server or dedicated server), the cleared input slot binding will replicate from
	 * the server to all clients, keeping the ability state synchronized across all connected players. This
	 * replication allows clients to accurately update their UI elements (like spell globes and ability bars)
	 * to reflect that the ability is no longer bound to its previous input slot. Without this call, clients
	 * wouldn't receive the updated ability spec until the next full ability array replication, causing desync
	 * between what the server knows and what clients see in their UI. We pass *Spec with the dereference operator to 
	 * convert the Spec pointer to a reference, since this function expects a reference rather than a pointer.
	 */
	MarkAbilitySpecDirty(*Spec);
}

void UFoxAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	/*
	 * Create a scoped lock on this ASC's ability list to ensure thread-safe iteration.
	 * FScopedAbilityListLock prevents the ability list from being modified (abilities added/removed/changed)
	 * while we're iterating through it. This is critical in multiplayer games where abilities can be granted
	 * or revoked from other threads (e.g., server replication, gameplay effects). The lock is automatically
	 * released when ActiveScopeLock goes out of scope at the end of this function.
	 * We pass *this with the dereference operator to convert the 'this' pointer to a reference, since the constructor
	 * expects UAbilitySystemComponent& rather than a pointer.
	 */
	FScopedAbilityListLock ActiveScopeLock(*this);

	/*
	 * Iterate through all activatable ability specs granted to this ASC.
	 * GetActivatableAbilities() returns a TArray of FGameplayAbilitySpec representing all abilities
	 * this component can potentially activate. We use a non-const reference (FGameplayAbilitySpec&) because
	 * we may need to modify specs by clearing their slot tags if they match the target Slot parameter.
	 * This allows us to find and clear all abilities currently equipped to the specified input slot.
	 */
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		/*
		 * Check if this ability spec has the specified slot tag in its dynamic tags.
		 * AbilityHasSlot() searches through the spec's dynamic tags to determine if it contains the Slot tag
		 * (e.g., "InputTag.LMB", "InputTag.1") that we're trying to clear. We pass a pointer to the current Spec
		 * using the address-of operator (&) and the Slot tag to check against. If this returns true, it means
		 * this ability is currently equipped to the input slot we want to clear, and we need to remove its
		 * slot tag in the next step.
		 */
		if (AbilityHasSlot(&Spec, Slot))
		{
			/*
			 * Remove the slot tag from this ability spec to clear its input binding.
			 * ClearSlot() extracts the input tag from the spec's dynamic tags and removes it, breaking the
			 * connection between this ability and the input slot. We pass a pointer to the Spec using the
			 * address-of operator (&) so ClearSlot can modify the spec's tags and mark it dirty for replication.
			 * This effectively unequips the ability from the specified input slot, making room for a different
			 * ability to be equipped there or leaving the slot empty.
			 */
			ClearSlot(&Spec);
		}
	}
}

bool UFoxAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	/*
	 * Iterate through all gameplay tags in the ability spec's dynamic tags container to search for a slot tag match.
	 * GetDynamicSpecSourceTags() returns a FGameplayTagContainer that holds tags dynamically added to this ability spec
	 * at runtime, including input slot tags (like "InputTag.LMB" or "InputTag.1") that were added when the ability was
	 * equipped. We iterate through each tag to check if any of them match the Slot parameter we're searching for.
	 * Each Tag in the container is copied during iteration (not a reference), which is acceptable for lightweight
	 * FGameplayTag structs.
	 */
	for (FGameplayTag Tag : Spec->GetDynamicSpecSourceTags())
	{
		/*
		 * Check if this tag exactly matches the Slot tag we're searching for.
		 * MatchesTagExact() performs an exact equality check. It returns true only if Tag is identical to Slot
		 * (not just a parent or child tag match). This ensures we find the specific input slot tag we're looking for,
		 * such as "InputTag.LMB" or "InputTag.1", without matching related tags like "InputTag" or "InputTag.RMB".
		 */
		if (Tag.MatchesTagExact(Slot))
		{
			/*
			 * Return true immediately to indicate that this ability spec has the specified slot tag.
			 * This early return exits the function as soon as we find a match. The true value signals to the caller 
			 * (ClearAbilitiesOfSlot) that this ability is currently equipped to the input slot being queried.
			 */
			return true;
		}
	}
	/*
	 * Return false to indicate that this ability spec does not have the specified slot tag.
	 * This return statement is reached only if we've iterated through all dynamic tags without finding an exact
	 * match to the Slot parameter. The false value signals to the caller that this ability is not currently
	 * equipped to the input slot being queried, so it should be left alone when clearing abilities from that slot.
	 * This distinguishes abilities equipped to different slots from the one being cleared.
	 */
	return false;
}

void UFoxAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	
	/*
	 * Check if startup abilities have not yet been marked as given on this client.
	 * This check is necessary because OnRep_ActivateAbilities() is called whenever the replicated ActivatableAbilities
	 * array changes on the server and replicates to this client. This can happen multiple times during gameplay
	 * (when new abilities are granted, removed, or modified), but we only want to broadcast the AbilitiesGivenDelegate
	 * once - the first time abilities are received from the server during initial replication.
	 * bStartupAbilitiesGiven acts as a one-time initialization guard to prevent duplicate broadcasts that could
	 * cause UI widgets or other listening systems to incorrectly reinitialize or display duplicate ability information.
	 */
	if (!bStartupAbilitiesGiven)
	{
		// Set the flag to true indicating that startup abilities have been received and processed on this client.
		bStartupAbilitiesGiven = true;
		
		/*
		 * Broadcast the AbilitiesGivenDelegate multicast delegate.
		 * This notifies all bound listeners (such as UI widgets or game systems) that abilities have been replicated
		 * from the server and are now available on this client. This is the client-side equivalent of the broadcast
		 * that occurs in AddCharacterAbilities() on the server, ensuring that client UI elements like ability bars
		 * can initialize and display ability information once the abilities have been replicated.
		 * Listeners can use this notification to query this ASC's abilities and update their displays accordingly.
		 */
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UFoxAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	/*
	 * Broadcast the AbilityStatusChanged multicast delegate to notify all registered listeners that an ability's
	 * status has changed.
	 * 
	 * This delegate broadcasts three key pieces of information:
	 * 1. AbilityTag: The gameplay tag identifying which specific ability's status changed (e.g., "Abilities.Fire",
	 *    "Abilities.Lightning"). This allows listeners to identify and update the correct ability in their displays.
	 * 2. StatusTag: The new status tag for the ability (e.g., "Abilities.Status.Eligible", "Abilities.Status.Equipped",
	 *    "Abilities.Status.Unlocked"). This tells listeners what the ability's current state is.
	 * 3. AbilityLevel: The integer representing the ability's current power level after the status change. This allows
	 *    listeners to update UI elements showing the ability's level, such as spell globe level displays in the spell menu.
	 */
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UFoxAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	/*
	 * Declare a local FGameplayTagContainer to hold the asset tags from the applied gameplay effect.
	 * FGameplayTagContainer is a collection class that efficiently stores and queries gameplay tags.
	 * This container will be populated with tags that describe the effect's properties (e.g., "Message.HealthPotion",
	 * "Message.Damage.Fire") which can be used by UI systems to display appropriate visual feedback or messages.
	 * We use a local variable here as a temporary holder to extract tags from the effect spec before broadcasting them.
	 */
	FGameplayTagContainer TagContainer;
	
	/*
	 * Populate the TagContainer with all asset tags from the applied gameplay effect.
	 * GetAllAssetTags() extracts tags from the EffectSpec that were defined in the gameplay effect's blueprint details panel
	 * under the "Asset Tags" section. These tags categorize and identify the effect type (e.g., damage, healing, buff).
	 * The function takes a reference to our TagContainer and fills it with all asset tags from the effect.
	 * Asset tags differ from granted tags (which are applied to the target) - asset tags describe the effect itself
	 * and are used for effect identification, filtering, and UI display purposes without affecting gameplay mechanics.
	 */
	EffectSpec.GetAllAssetTags(TagContainer);

	/*
	 * Broadcast the EffectAssetTags multicast delegate, passing the populated TagContainer to all bound listeners.
	 * This notifies all systems that have registered callbacks with this delegate (typically UI widgets like
	 * overlay controllers) that a gameplay effect has been applied with these specific asset tags.
	 * Listeners can use these tags to trigger visual effects, display messages, update health bars, or perform
	 * any other logic that needs to respond to effects being applied to this character.
	 * Broadcast() calls all bound functions synchronously in the order they were registered, passing the TagContainer
	 * by const reference to each callback for efficient read-only access to the effect's identifying tags.
	 */
	EffectAssetTags.Broadcast(TagContainer);
}
