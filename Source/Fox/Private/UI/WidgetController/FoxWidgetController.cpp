// Copyright TryingToMakeGames


#include "UI/WidgetController/FoxWidgetController.h"

#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/FoxPlayerController.h"
#include "Player/FoxPlayerState.h"

void UFoxWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

// The next two functions are empty because we only want to implement them in the child of this class.
void UFoxWidgetController::BroadcastInitialValues()
{
	
}

void UFoxWidgetController::BindCallbacksToDependencies()
{
	
}

void UFoxWidgetController::BroadcastAbilityInfo()
{
	// Uses the getter function for the FoxAbilitySystemComponent and checks if startup abilities have been granted
	// using the bool member variable. If not, return early.
	if (!GetFoxASC()->bStartupAbilitiesGiven) return;
	
	/*
	 * Declare a LOCAL delegate variable (not a member variable) of type FForEachAbility.
	 * 
	 * FForEachAbility is a delegate TYPE declared in FoxAbilitySystemComponent.h using DECLARE_DELEGATE_OneParam.
	 * BroadcastDelegate is a local INSTANCE of that delegate type, created on the stack in this function.
	 * 
	 * Why is this a local variable and not a member variable in the .h file?
	 * - This delegate is only needed temporarily during this single function call for iteration purposes
	 * - It follows the "callback parameter" pattern where a delegate instance is passed to an iteration function
	 * - Member delegates (like EffectAssetTags, AbilitiesGivenDelegate) are for persistent multi-subscriber 
	 *   event broadcasting, while this is a one-time, single-subscriber callback for iteration
	 * - Creating it locally keeps the class interface clean and makes the temporary nature explicit
	 */
	FForEachAbility BroadcastDelegate;

	/*
	 * Bind a lambda function to the BroadcastDelegate using BindLambda().
	 * This lambda will be executed once for each ability when ForEachAbility() calls Execute() on the delegate.
	 * 
	 * Lambda capture list [this]:
	 *   - 'this' captures the current UOverlayWidgetController instance, allowing access to member variables
	 *     like AbilityInfo and AbilityInfoDelegate inside the lambda
	 * 
	 * Lambda parameter (const FGameplayAbilitySpec& AbilitySpec):
	 *   - Each granted ability is represented by a FGameplayAbilitySpec struct containing:
	 *     * The ability class
	 *     * Ability level
	 *     * Ability tags (identifying the ability type)
	 *     * Dynamic tags (including input binding tags)
	 *     * Activation state
	 *   - ForEachAbility() will pass each ability's spec to this lambda when Execute() is called
	 */
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		/*
		 * Look up the display information (UI data) for this ability from the AbilityInfo data asset.
		 * 
		 * GetAbilityTagFromSpec() extracts the ability's identifying tag from the AbilitySpec
		 * (e.g., "Abilities.Fire", "Abilities.Lightning"). This tag uniquely identifies the ability type.
		 * 
		 * FindAbilityInfoForTag() searches the AbilityInfo data asset (UAbilityInfo*, set in blueprint)
		 * for a row matching this ability tag, returning a FFoxAbilityInfo struct likely containing:
		 *   - Ability icon/image
		 *   - Ability name/description
		 *   - Cooldown information
		 *   - Cost information
		 *   - Background image
		 *   - Ability tag (for identification)
		 *   - Input tag (will be overwritten in next line)
		 * 
		 * The values of the variables of the data asset are set in blueprint. This separates ability logic (in the 
		 * ability class) from ability display data (in the data asset), allowing designers to modify UI appearance
		 * without touching C++ code.
		 */
		FFoxAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(FoxAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));

		/*
		 * Set the value of the Info struct's InputTag field with the actual input binding for this ability.
		 * 
		 * GetInputTagFromSpec() extracts the input tag from AbilitySpec.DynamicAbilityTags
		 * (e.g., "InputTag.LMB", "InputTag.1", "InputTag.2"). This tag was added to DynamicAbilityTags
		 * in AddCharacterAbilities() when the ability was granted.
		 * 
		 * The InputTag tells the UI which input key/button this ability is bound to, allowing the UI
		 * to display the correct key binding on the ability icon (e.g., showing "LMB" or "1" on the icon).
		 * 
		 * We overwrite the InputTag here because the data asset might have a default/placeholder value,
		 * but the actual input binding is determined at runtime when abilities are granted and can vary
		 * per character or player configuration.
		 */
		Info.InputTag = FoxAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		
		/*
		 * Set the value of the Info struct's StatusTag field with the actual status for this ability.
		 * 
		 * GetStatusFromSpec() extracts the status tag from the AbilitySpec
		 * (e.g., "Abilities.Status.Locked", "Abilities.Status.Unlocked", "Abilities.Status.Equipped").
		 * This tag represents the current state of the ability for this character.
		 * 
		 * The StatusTag tells the UI what state to display for this ability, allowing the UI
		 * to show whether the ability is locked, unlocked but not equipped, or currently equipped.
		 * 
		 * We overwrite the StatusTag here because the data asset might have a default/placeholder value,
		 * but the actual status is determined at runtime based on the character's progression and can
		 * change as the player unlocks and equips different abilities.
		 */
		Info.StatusTag = FoxAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);

		/* 
		 * AbilityInfoDelegate is a multicast delegate (declared in the header) that UI widgets bind to in order to receive
		 * ability data for display.
		 * 
		 * When broadcast, all bound UI widgets (e.g., ability bar, ability tooltips) receive the
		 * FFoxAbilityInfo struct containing the ability's icon, name, input binding, etc., and can
		 * update their display accordingly (showing the ability icon, key binding, cooldown state, etc.).
		 * 
		 * This broadcast happens once per ability during initialization, populating the UI with all
		 * available abilities.
		 */
		AbilityInfoDelegate.Broadcast(Info);
	});
	
	/*
	 * We use the getter function for the FoxAbilitySystemComponent, but we could really just use the member variable directly.
	 * We do this simply to stay as close to the tutorial as possible.
	 *
	 * ForEachAbility() is defined in UFoxAbilitySystemComponent and loops through the ASC's
	 * ActivatableAbilities.Items array. For each FGameplayAbilitySpec in that array, it calls
	 * BroadcastDelegate.Execute(AbilitySpec), which invokes the lambda function we bound above.
	 * 
	 * This results in the lambda executing once per ability, looking up each ability's display info
	 * and broadcasting it to UI widgets. By the end of this call, all ability information has been
	 * sent to the UI.
	 */
	GetFoxASC()->ForEachAbility(BroadcastDelegate);
}


AFoxPlayerController* UFoxWidgetController::GetFoxPC()
{
	// Check if the cached Fox player controller is null (first call or not yet cached)
	if (FoxPlayerController == nullptr)
	{
		// Cast the base player controller to our Fox-specific player controller type and cache it
		FoxPlayerController = Cast<AFoxPlayerController>(PlayerController);
	}
	// Return the cached Fox player controller
	return FoxPlayerController;
}

AFoxPlayerState* UFoxWidgetController::GetFoxPS()
{
	// Check if the cached Fox player state is null (first call or not yet cached)
	if (FoxPlayerState == nullptr)
	{
		// Cast the base player state to our Fox-specific player state type and cache it
		FoxPlayerState = Cast<AFoxPlayerState>(PlayerState);
	}
	// Return the cached Fox player state
	return FoxPlayerState;
}

UFoxAbilitySystemComponent* UFoxWidgetController::GetFoxASC()
{
	// Check if the cached Fox ability system component is null (first call or not yet cached)
	if (FoxAbilitySystemComponent == nullptr)
	{
		// Cast the base ability system component to our Fox-specific ASC type and cache it
		FoxAbilitySystemComponent = Cast<UFoxAbilitySystemComponent>(AbilitySystemComponent);
	}
	// Return the cached Fox ability system component
	return FoxAbilitySystemComponent;
}

UFoxAttributeSet* UFoxWidgetController::GetFoxAS()
{
	// Check if the cached Fox attribute set is null (first call or not yet cached)
	if (FoxAttributeSet == nullptr)
	{
		// Cast the base attribute set to our Fox-specific attribute set type and cache it
		FoxAttributeSet = Cast<UFoxAttributeSet>(AttributeSet);
	}
	// Return the cached Fox attribute set
	return FoxAttributeSet;
}
