// Copyright TryingToMakeGames


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	
	// Instantiate a new UWaitCooldownChange object using Unreal's object creation system (NewObject template function).
	// This creates a UObject-derived instance that's automatically tracked by Unreal's garbage collection system,
	// ensuring proper memory management. The object will persist until explicitly marked for garbage collection via
	// MarkAsGarbage() or when no references to it remain.
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	
	// Store the ability system component passed into this function
	WaitCooldownChange->ASC = AbilitySystemComponent;

	// Store the cooldown tag passed into this function
	WaitCooldownChange->CooldownTag = InCooldownTag;

	// Perform validation checks on the input parameters to ensure they're in a valid state before proceeding with
	// delegate registration. IsValid() checks if the AbilitySystemComponent pointer is non-null and points to a valid
	// UObject that hasn't been marked for garbage collection or destroyed. IsValid() on the gameplay tag checks if the
	// tag has been properly initialized with a valid tag name.
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		// Clean up the newly created async task object by calling EndTask(), which unregisters any delegates (though
		// none should be registered yet at this point in execution), calls SetReadyToDestroy() to signal to Unreal's
		// async task system that this task is finished, and calls MarkAsGarbage() to tell the garbage collector this
		// object is no longer needed and can be cleaned up in the next GC pass. This prevents memory leaks and ensures
		// we don't leave a partially-initialized async task object floating around in memory that could never complete
		// or be properly cleaned up. We then return nullptr to indicate to the caller that the operation failed and no
		// valid async task was created.
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	/*
	 * Register a callback for cooldown tag count changes by calling RegisterGameplayTagEvent on the ability system
	 * component. This function takes two parameters: the gameplay tag to monitor (InCooldownTag) and the type of
	 * event to listen for (EGameplayTagEventType::NewOrRemoved, which triggers when the tag count changes from 0 to
	 * any value or from any value to 0). The function returns a reference to a multicast delegate, which we
	 * immediately bind to by calling AddUObject.
	 * 
	 * AddUObject is a template function that binds a UObject member function to the delegate - in this case, it binds
	 * CooldownTagChanged (a member function of this class UWaitCooldownChange) to be called whenever the cooldown tag
	 * is added to or removed from the ability system component. The first parameter to AddUObject is the UObject
	 * instance (WaitCooldownChange) that owns the callback function, and the second parameter is a pointer to the
	 * member function (&UWaitCooldownChange::CooldownTagChanged) that will be invoked.
	 *
	 * We must explicitly pass WaitCooldownChange as the first parameter because we're inside a static function
	 * (WaitForCooldownChange), and static functions don't have access to the implicit 'this' pointer. In C++, 'this'
	 * is an implicit pointer to the current instance of a class, but it only exists within non-static member
	 * functions - functions that are called on a specific object instance. Static functions, by contrast, belong to
	 * the class itself rather than to any particular instance, so there is no 'this' pointer to reference. Since
	 * we're in a static context here, we can't write 'this' or use any implicit reference to the current object.
	 * Instead, we must explicitly pass the WaitCooldownChange pointer (the instance we created with NewObject earlier
	 * in this function) so AddUObject knows which specific object instance should receive the callback when the
	 * delegate fires.
	 */
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UWaitCooldownChange::CooldownTagChanged);

	// Register a callback for when new gameplay effects are added to the ability system component by binding to the
	// OnActiveGameplayEffectAddedDelegateToSelf multicast delegate. This delegate is a member of
	// UAbilitySystemComponent and broadcasts whenever a gameplay effect is applied to the component. We use AddUObject
	// to bind the OnActiveEffectAdded member function of our WaitCooldownChange instance to this delegate. When a
	// gameplay effect is applied, OnActiveEffectAdded will be called and its input parameters will be passed to it from
	// the broadcasting delegate.
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	// Return the fully configured async task object to the caller (typically a Blueprint graph). At this point, the
	// object has been instantiated, validated, and has registered two delegate callbacks: one for cooldown tag count
	// changes and one for new gameplay effects being added. The object is now actively monitoring the ability system
	// component and will broadcast through its CooldownStart and CooldownEnd delegates when relevant events occur,
	// triggering execution on the corresponding output pins of the Blueprint async node.
	return WaitCooldownChange;
}

void UWaitCooldownChange::EndTask()
{
	
	// Validate that the ability system component is still valid (not null and not marked for destruction) before
	// attempting to unregister delegates from it. If the component has already been destroyed or is invalid, we
	// return early to avoid crashes from attempting to access a destroyed object.
	if (!IsValid(ASC)) return;

	/*
	 * Unregister the cooldown tag change callback that was previously registered in WaitForCooldownChange().
	 * 
	 * 1. ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
	 *    This part calls RegisterGameplayTagEvent on the ability system component with the same parameters we used
	 *    during registration (the cooldown tag and the NewOrRemoved event type). However, despite the name
	 *    "Register", we're not actually registering anything new here - we're retrieving a reference to the existing
	 *    multicast delegate that manages callbacks for this specific tag and event type combination. The
	 *    RegisterGameplayTagEvent function returns a reference to an FOnGameplayEffectTagCountChanged delegate
	 *    (which is a multicast delegate type), and this delegate is the same one we bound our callback to earlier.
	 * 
	 * 2. .RemoveAll(this)
	 *    Once we have the delegate reference, we immediately call RemoveAll() on it, passing 'this' as the parameter.
	 *    RemoveAll() is a method available on all Unreal multicast delegates that removes ALL delegate bindings
	 *    associated with a specific UObject instance. The 'this' pointer refers to the current UWaitCooldownChange
	 *    instance, so RemoveAll(this) will find and remove any callbacks bound to member functions of this specific
	 *    object instance. In our case, this removes the binding to CooldownTagChanged() that was created during
	 *    initialization via AddUObject().
	 * 
	 * This cleanup is critical because delegates in Unreal don't automatically remove bindings when the bound object
	 * is destroyed. If we didn't explicitly remove these bindings, the ability system component would still attempt to
	 * invoke our callbacks even after this UWaitCooldownChange object has been marked for garbage collection and
	 * potentially destroyed, resulting in crashes or undefined behavior when the delegate tries to call a member
	 * function on a destroyed object.
	 */
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	// Signal to Unreal's async task system that this task has completed its work and is ready to be destroyed. This
	// call is part of the UBlueprintAsyncActionBase interface and notifies any systems tracking this async action that
	// it has finished executing.
	SetReadyToDestroy();

	// Mark this object for garbage collection by Unreal's garbage collector. This tells the GC that this object is no
	// longer needed and can be deallocated during the next garbage collection pass. Until the GC actually runs and
	// destroys this object, it remains in memory but is flagged as "garbage" and won't be considered by most Unreal
	// systems. This is the proper way to destroy UObject-derived instances in Unreal Engine rather than using delete.
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	// Check if the cooldown tag count has reached zero, indicating that no gameplay effects with this cooldown tag are
	// currently active on the ability system component. When NewCount is 0, it means the cooldown tag has been completely
	// removed from the ASC (all gameplay effects granting this tag have expired or been removed), signaling that the
	// cooldown period has ended.
	if (NewCount == 0)
	{
		// Broadcast the CooldownEnd delegate to notify any Blueprint nodes or C++ code listening to this event that the
		// cooldown has finished. We pass 0.f as the time remaining parameter since the cooldown has completely ended
		// (zero time remaining). In Blueprint, this will trigger execution on the "Cooldown End" output pin of the
		// WaitForCooldownChange async node, allowing Blueprint graphs to respond to the cooldown completion by executing
		// whatever logic is connected to that pin (such as playing a UI notification).
		CooldownEnd.Broadcast(0.f);
	}
}

void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC,
	const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	
	// Create a container to hold the asset tags from the applied gameplay effect spec. Asset tags are metadata tags
	// assigned directly to the gameplay effect asset itself (set in the GameplayEffect's AssetTags array in the editor)
	// and are typically used to categorize or identify the type of effect (e.g., "Damage", "Buff", "Debuff").
	FGameplayTagContainer AssetTags;
	
	// Retrieve all asset tags from the gameplay effect spec that was just applied and store them in the AssetTags container
	SpecApplied.GetAllAssetTags(AssetTags);
	
	// Create a container to hold the granted tags from the applied gameplay effect spec. Granted tags are gameplay tags
	// that the effect actively applies to the target actor while the effect is active (configured in the GameplayEffect's
	// GrantedTags section). These tags are added to the target's ability system component when the effect is applied and
	// removed when the effect expires. For example, a stun effect might grant a "Status.Stunned" tag to mark the target
	// as stunned.
	FGameplayTagContainer GrantedTags;
	
	// Retrieve all tags that this gameplay effect grants to its target and store them in the GrantedTags container
	SpecApplied.GetAllGrantedTags(GrantedTags);
	
	// Check if the cooldown tag we're monitoring exists in either the asset tags or the granted tags of the applied
	// gameplay effect. We use HasTagExact() rather than HasTag() to perform an exact match - this means we only match
	// the specific tag "Ability.Cooldown.Fire" and not parent or child tags like "Ability.Cooldown". This check
	// determines whether the newly applied gameplay effect is actually a cooldown effect that we care about monitoring.
	// If the cooldown tag is found in either location, we proceed.
	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		
		// Create a gameplay effect query object to find all active cooldown effects that have the monitored cooldown tag
		// in their "owning tags" (also called DynamicGrantedTags). Owning tags are gameplay tags dynamically added to a
		// gameplay effect instance at runtime, typically during the effect's creation or application. Unlike asset tags
		// (static metadata set on the GameplayEffect asset in the editor) and granted tags (tags the effect applies to
		// its target), owning tags belong to the effect instance itself and can be set programmatically when creating
		// the effect spec. For example, a cooldown effect might have its cooldown category (like "Ability.Cooldown.Fire")
		// added as an owning tag when the ability is activated, allowing the system to query and identify which specific
		// cooldown effect is active. MakeQuery_MatchAnyOwningTags is a static factory function that constructs a query
		// matching effects where any of the specified tags are present in the effect's owning tags, and
		// GetSingleTagContainer() wraps our single CooldownTag in a FGameplayTagContainer since the query function
		// expects a container rather than an individual tag.
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		
		// Query the ability system component to get an array of remaining durations (in seconds) for all active gameplay
		// effects that match our query. GetActiveEffectsTimeRemaining takes the query we just constructed and returns a
		// TArray<float> where each element represents the time remaining on a matching gameplay effect. If multiple
		// cooldown effects are active (e.g., ability was somehow triggered multiple times), this array will contain
		// multiple time values. If no matching effects are found, the array will be empty.
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		
		// Check if we found any active cooldown effects by testing if the TimesRemaining array contains at least one
		// element (Num() > 0). If the array is empty, it means no cooldown effects are currently active, which would be
		// unexpected since we only reach this code when a cooldown effect was just applied. However, this check protects
		// against edge cases or timing issues.
		if (TimesRemaining.Num() > 0)
		{
			// Initialize our time remaining variable with the first cooldown duration from the array. This serves as our
			// starting point for finding the longest remaining cooldown. We access the first element using [0] since we've
			// already verified the array has at least one element in the if condition above.
			float TimeRemaining = TimesRemaining[0];
			
			
			// Iterate through all remaining cooldown times to find the longest one. This handles the edge case where multiple
			// cooldown effects with the same cooldown tag might be active simultaneously (e.g., if an ability was triggered
			// multiple times in rapid succession or if cooldown reduction mechanics caused overlapping cooldowns). We start
			// the loop at index 0 (redundantly checking the first element we already stored in TimeRemaining) and compare each
			// duration in the array to find the maximum value. This ensures we always broadcast the longest remaining cooldown
			// time, which represents when the ability will actually become available again.
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				// Compare the current array element's cooldown time against our current maximum. If this cooldown has more
				// time remaining than our current maximum, update TimeRemaining to store this larger value instead.
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}
			
			// Broadcast the CooldownStart delegate to notify any listening Blueprint nodes or C++ code that a cooldown has
			// begun. We pass the longest remaining cooldown time found in the loop above as the parameter. In Blueprint, this
			// triggers execution on the "Cooldown Start" output pin of the WaitForCooldownChange async node, allowing
			// connected Blueprint logic to respond (such as starting a UI cooldown timer). The
			// time value can be used to display accurate cooldown information to the player.
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
