// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "FoxAbilitySystemComponent.generated.h"

class UFoxAbilitySystemComponent;

// whats the difference between dynamic and non dynamic delegates here and multicast vs non multicast
// whats the effect of the const and reference symbol. Why do we not put names and only types for the arguments the 
// callbacks that will bind to this need to take?
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* Asset Tags*/)

// DECLARE_MULTICAST_DELEGATE_ creates a multicast delegate type that can broadcast to multiple bound functions.
// FAbilitiesGiven - the name of the delegate type being declared
// Bound functions must accept no input parameters, because this delegate does not pass any to functions that bind to it
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);

// DECLARE_DELEGATE_OneParam creates a single-cast delegate type (unlike multicast, only one function can be bound at a time).
// First parameter: FForEachAbility - the name of the delegate type being declared
// Second parameter: const FGameplayAbilitySpec& - the type of the single parameter that will be passed to the bound callback
// This delegate is used for iteration callbacks in ForEachAbility(), where a single function is executed for each ability spec.
// Bound functions must accept a const reference to FGameplayAbilitySpec as their parameter.
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);

// DECLARE_MULTICAST_DELEGATE_ThreeParams creates a multicast delegate type that can broadcast to multiple bound functions.
// FAbilityStatusChanged - the name of the delegate type being declared
// First parameter: const FGameplayTag& - the ability tag identifying which ability's status changed
// Second parameter: const FGameplayTag& - the new status tag for the ability (e.g., "Abilities.Status.Locked", "Abilities.Status.Unlocked")
// Third parameter: int32 - the current level of the ability whose status changed
// This delegate is used to notify systems (like UI) when an ability's status changes due to leveling up, unlocking, or equipping.
// Bound functions must accept two const references to FGameplayTag and one int32 as their parameters.
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/)

// DECLARE_MULTICAST_DELEGATE_FourParams creates a multicast delegate type that can broadcast to multiple bound functions.
// FAbilityEquipped - the name of the delegate type being declared
// First parameter: const FGameplayTag& - the ability tag identifying which ability was equipped
// Second parameter: const FGameplayTag& - the status tag of the ability at the time of equipping (e.g., "Abilities.Status.Equipped")
// Third parameter: const FGameplayTag& - the slot tag where the ability was equipped (e.g., "InputTag.1", "InputTag.LMB")
// Fourth parameter: const FGameplayTag& - the previous slot tag where the ability was equipped before, or empty tag if newly equipped
// This delegate is used to notify systems (like UI) when an ability is equipped to an input slot, allowing updates to ability bar displays.
// Bound functions must accept four const references to FGameplayTag as their parameters.
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);

/**
 * 
 */
UCLASS()
class FOX_API UFoxAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	
	/**
	 * Initializes the ability system component's actor info and binds delegates.
	 * This should be called after the ASC's actor info has been set.
	 * Binds the OnGameplayEffectAppliedDelegateToSelf to ClientEffectApplied for effect tracking.
	 */
	void AbilityActorInfoSet();

	/**
	 * Multicast delegate that broadcasts gameplay effect asset tags when effects are applied to this component.
	 * Bound callbacks receive a const reference to a FGameplayTagContainer containing the applied effect's asset tags.
	 * Used to notify UI or other systems about effects being applied (e.g., for displaying buff/debuff icons).
	 */
	FEffectAssetTags EffectAssetTags;

	/**
	 * Multicast delegate that broadcasts when startup abilities have been granted to this component.
	 * Bound callbacks receive a pointer to this UFoxAbilitySystemComponent.
	 * Used to notify systems (like UI) that abilities are now available and ready to be displayed or used.
	 */
	FAbilitiesGiven AbilitiesGivenDelegate;
	
	/**
	 * Multicast delegate instance of the type defined above that broadcasts when an ability's status changes.
	 * Bound callbacks receive the ability tag and the new status tag.
	 * Used to notify systems (like UI) when abilities are unlocked, locked, equipped, or otherwise change state
	 * due to player progression or level changes. For example, when a player levels up and an ability becomes
	 * available, this delegate broadcasts the ability's tag along with its new "Eligible" status tag.
	 */
	FAbilityStatusChanged AbilityStatusChanged;

	/**
	 * Multicast delegate instance that broadcasts when an ability is equipped to an input slot.
	 * Bound callbacks receive the ability tag, status tag, slot tag, and previous slot tag.
	 * Used to notify systems (like UI) when abilities are assigned to or moved between input slots,
	 * allowing the ability bar and spell menu to update their displays. For example, when a player
	 * drags an ability to a new slot in the UI, this delegate broadcasts the change so all relevant
	 * widgets can update their button icons and ability information to reflect the new equipment state.
	 */
	FAbilityEquipped AbilityEquipped;

	/**
	 * Grants an array of gameplay abilities to this ability system component.
	 * Each ability is granted with level 1 and its StartupInputTag is added to the ability spec's dynamic tags
	 * to enable input-based ability activation through the AbilityInputTagHeld/Released methods.
	 * Sets bStartupAbilitiesGiven to true and broadcasts AbilitiesGivenDelegate when complete.
	 * 
	 * @param StartupAbilities Array of UGameplayAbility classes to grant to the owner of this ASC
	 */
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	
	/**
	 * Grants an array of passive gameplay abilities to this ability system component.
	 * Passive abilities are automatically activated upon being granted and remain active throughout the character's lifetime.
	 * Each ability is granted with level 1 and its activation policy should be set to OnGranted or similar to ensure
	 * immediate activation. Unlike active abilities, passive abilities typically don't require input tags for activation
	 * as they activate themselves automatically when granted.
	 * 
	 * @param StartupPassiveAbilities Array of passive UGameplayAbility classes to grant to the owner of this ASC
	*/
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

	/**
	 * Flag indicating whether startup abilities have been granted to this component.
	 * Set to true after AddCharacterAbilities() completes.
	 * Used to prevent duplicate ability grants and to signal that the component is fully initialized.
	 */
	bool bStartupAbilitiesGiven = false;
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Handles input press/hold events for abilities.
	 * Searches through granted abilities for one with a dynamic tag matching the InputTag,
	 * then attempts to activate that ability if it's not already active.
	 * Called by the player controller when an input action is pressed or held.
	 * 
	 * @param InputTag The gameplay tag representing the input action (e.g., "InputTag.LMB" or "InputTag.1")
	 */
	void AbilityInputTagHeld(const FGameplayTag& InputTag);

	/**
	 * Handles input release events for abilities.
	 * Searches through granted abilities for one with a dynamic tag matching the InputTag,
	 * then notifies the ability spec that its input has been released.
	 * Called by the player controller when an input action is released.
	 * 
	 * @param InputTag The gameplay tag representing the input action (e.g., "InputTag.LMB" or "InputTag.1")
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	/**
	 * Iterates through all granted gameplay abilities and invokes the provided delegate for each ability spec.
	 * "Executes a delegate" here means calling Execute() on the delegate parameter passed to this function,
	 * not broadcasting a member delegate. For each ability, this function calls Delegate.Execute(AbilitySpec).
	 * The Execute() method is provided by the delegate type and directly invokes the single function bound to this
	 * delegate, passing the AbilitySpec as an argument. Unlike Broadcast() (used for multicast delegates),
	 * Execute() is for single-cast delegates and calls the one bound function synchronously.
	 * Useful for performing operations on all abilities, such as populating UI elements or querying ability states.
	 * The delegate receives a const reference to each FGameplayAbilitySpec during iteration.
	 * 
	 * @param Delegate The callback delegate to invoke for each ability spec
	 */
	void ForEachAbility(const FForEachAbility& Delegate);

	/**
	 * Extracts the ability tag from a gameplay ability spec's ability tags container.
	 * Returns the first tag found in the spec's ability tags, or an empty tag if none exist.
	 * This tag typically identifies the ability type (e.g., "Abilities.Fire" or "Abilities.Jump").
	 * 
	 * @param AbilitySpec The gameplay ability spec to extract the ability tag from
	 * @return The ability's identifying gameplay tag, or an empty tag if none found
	 */
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	/**
	 * Extracts the input tag from a gameplay ability spec's dynamic tags container.
	 * Returns the first tag found in the spec's dynamic tags, or an empty tag if none exist.
	 * This tag represents the input binding for the ability (e.g., "InputTag.LMB" or "InputTag.1").
	 * 
	 * @param AbilitySpec The gameplay ability spec to extract the input tag from
	 * @return The ability's input binding gameplay tag, or an empty tag if none found
	 */
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	
	/**
	 * Extracts the status tag from a gameplay ability spec's ability tags container.
	 * Returns the status tag associated with the ability, or an empty tag if none exist.
	 * This tag typically represents the current state or status of the ability (e.g., "Abilities.Status.Locked" or "Abilities.Status.Unlocked").
	 * 
	 * @param AbilitySpec The gameplay ability spec to extract the status tag from
	 * @return The ability's status gameplay tag, or an empty tag if none found
	 */
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	
	/**
	 * Retrieves the current status tag for an ability with the specified ability tag.
	 * This function searches through all granted abilities to find the ability spec matching the specified ability tag,
	 * then extracts and returns its status tag (e.g., "Abilities.Status.Locked", "Abilities.Status.Unlocked", "Abilities.Status.Eligible").
	 * The status tag indicates the ability's current state, which determines whether it can be activated, upgraded, or equipped.
	 * Returns an empty tag if the ability is not found.
	 * 
	 * @param AbilityTag The gameplay tag identifying the ability to query (e.g., "Abilities.Fire.FireBolt")
	 * @return The ability's current status gameplay tag, or an empty tag if the ability is not found
	 */
	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * Retrieves the input tag bound to an ability with the specified ability tag.
	 * This function searches through all granted abilities to find the ability spec matching the specified ability tag,
	 * then extracts and returns its input tag from the dynamic tags (e.g., "InputTag.LMB", "InputTag.1").
	 * The input tag represents which input action is bound to activate this ability.
	 * Returns an empty tag if the ability is not found or has no input binding.
	 * 
	 * @param AbilityTag The gameplay tag identifying the ability to query (e.g., "Abilities.Fire.FireBolt")
	 * @return The ability's input binding gameplay tag, or an empty tag if the ability is not found or unbound
	 */
	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);
	
	/**
	 * Function that searches through all granted abilities to find the ability spec matching the specified ability tag.
	 * Returns a pointer to the matching FGameplayAbilitySpec if found, or nullptr if no matching ability exists.
	 * This function is useful for querying or modifying a specific ability's state, such as checking its status,
	 * updating its level, or accessing its dynamic tags.
	 * 
	 * @param AbilityTag The gameplay tag identifying the ability to search for (e.g., "Abilities.Fire")
	 * @return Pointer to the matching ability spec, or nullptr if not found
	 */
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	
	/**
	 * Initiates an attribute upgrade request for the specified attribute.
	 * This function is called on the client and forwards the upgrade request to the server via the ServerUpgradeAttribute
	 * function. The AttributeTag identifies which attribute should be upgraded (e.g., "Attributes.Primary.Strength").
	 * The actual upgrade logic and validation is performed on the server to prevent cheating.
	 */
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	/**
	 * Server RPC that processes and validates attribute upgrade requests from clients.
	 * UFUNCTION(Server, Reliable) ensures this function executes on the server when called from a client.
	 * "Reliable" guarantees the RPC will arrive even under poor network conditions.
	 * This function validates that the player has sufficient resources (e.g., attribute points)
	 * and apply the attribute upgrade through gameplay effects or direct attribute modification.
	 * The implementation function will be auto-generated as ServerUpgradeAttribute_Implementation.
	 * 
	 * @param AttributeTag The gameplay tag identifying which attribute to upgrade
	 */
	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);
	
	/**
	 * Updates the status tags of all granted abilities based on the character's current level. This should be called on
	 * the server, as ability status changes should be server authoritative.
	 * 
	 * Iterates through all abilities and checks their level requirements against the provided level parameter.
	 * If an ability's level requirement is met, its status may be updated from "Locked" to "Eligible".
	 * Also, broadcasts the AbilityStatusChanged delegate for each ability that changes status,
	 * allowing UI and other systems to react to newly available abilities.
	 * 
	 * @param Level The character's current level used to determine which abilities should be unlocked
	 */
	void UpdateAbilityStatuses(int32 Level);
	
	/**
	 * Server RPC that processes and validates spell point spending requests from clients to upgrade or unlock abilities.
	 * UFUNCTION(Server, Reliable) ensures this function executes on the server when called from a client.
	 * "Reliable" guarantees the RPC will arrive even under poor network conditions.
	 * This function is called when the player clicks the "Spend Point" button in the spell menu UI (via 
	 * SpellMenuWidgetController::SpendPointButtonPressed). It validates that the player has sufficient spell points
	 * and that the ability's current status allows for spending points (e.g., status is "Eligible" or "Unlocked").
	 * Upon successful validation, it deducts a spell point, upgrades the ability (typically increasing its level),
	 * updates the ability's status tags if necessary, and replicates these changes to clients.
	 * The implementation function will be auto-generated as ServerSpendSpellPoint_Implementation.
	 * 
	 * @param AbilityTag The gameplay tag identifying which ability to spend a spell point on (e.g., "Abilities.Fire.FireBolt")
	 */
	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);
	
	/**
	 * Server RPC that processes and validates ability equip requests from clients.
	 * UFUNCTION(Server, Reliable) ensures this function executes on the server when called from a client.
	 * "Reliable" guarantees the RPC will arrive even under poor network conditions.
	 * This function is called when the player attempts to equip an ability to an input slot via the spell menu UI.
	 * It validates that the ability exists, is in an equippable state (e.g., status is "Unlocked" or "Equipped"),
	 * and handles slot management (clearing previous slot assignments, preventing duplicate slot usage).
	 * Upon successful validation, it assigns the ability to the specified slot by updating the ability spec's dynamic tags,
	 * updates the ability's status tag if necessary, and calls ClientEquipAbility to notify the client of the equipment change.
	 * The implementation function will be auto-generated as ServerEquipAbility_Implementation.
	 * 
	 * @param AbilityTag The gameplay tag identifying which ability to equip (e.g., "Abilities.Fire.FireBolt")
	 * @param Slot The gameplay tag identifying which input slot to equip the ability to (e.g., "InputTag.1", "InputTag.LMB")
	 */
	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);

	/**
	 * Notifies the UI about an ability equipment change by broadcasting the AbilityEquipped delegate.
	 * This function is called after ServerEquipAbility successfully processes an equip request on the server.
	 * It broadcasts the equipment change to all bound listeners (such as SpellMenuWidgetController and OverlayWidgetController)
	 * so they can update their displays to reflect the new ability-to-slot assignments.
	 * Unlike most client RPC functions, this is not marked with UFUNCTION(Client, Reliable) because it's called directly
	 * on both server and client after equipment operations complete, rather than being remotely invoked.
	 * 
	 * Example of remote invocation vs direct calling:
	 * - Remote invocation (typical Client RPC): ServerEquipAbility_Implementation() calls ClientUpdateAbilityStatus(),
	 *   which causes Unreal's networking system to automatically invoke ClientUpdateAbilityStatus_Implementation()
	 *   on the owning client's machine across the network. The function executes on a different machine than where it was called.
	 * - Direct calling (this function): ServerEquipAbility_Implementation() directly calls ClientEquipAbility()
	 *   in the same process/machine where it's running (the server), then separately the client also calls it locally
	 *   when needed. No automatic network transmission occurs. It's just a regular C++ function call within the same executable.
	 * 
	 * @param AbilityTag The gameplay tag identifying which ability was equipped (e.g., "Abilities.Fire.FireBolt")
	 * @param Status The status tag of the ability after equipping (typically "Abilities.Status.Equipped")
	 * @param Slot The gameplay tag identifying which input slot the ability was equipped to (e.g., "InputTag.1")
	 * @param PreviousSlot The gameplay tag identifying the slot the ability was previously equipped to, or empty tag if newly equipped
	 */
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);
	
	/**
	 * Retrieves the current level description and next level description for a specified ability.
	 * This function searches for the ability spec matching the provided AbilityTag, then calls the ability's
	 * GetDescription() and GetNextLevelDescription() methods to populate the output parameters with formatted
	 * description strings. These descriptions are typically displayed in the spell menu UI to show players
	 * what the ability does at its current level and what improvements the next level would provide.
	 * Used by SpellMenuWidgetController to update UI text when abilities are selected or their levels change.
	 * 
	 * @param AbilityTag The gameplay tag identifying which ability to retrieve descriptions for (e.g., "Abilities.Fire.FireBolt")
	 * @param OutDescription Output parameter that will be populated with the ability's current level description string
	 * @param OutNextLevelDescription Output parameter that will be populated with the ability's next level description string
	 * @return True if the ability was found and descriptions were successfully retrieved, false otherwise
	 */
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);
	
	/**
	 * Removes the input slot tag from the specified ability spec's dynamic tags.
	 * This function searches through the ability spec's dynamic tags to find and remove any input slot tag
	 * (tags matching "InputTag.*" pattern). This effectively unequips the ability from its current input slot
	 * without assigning it to a new slot. Used internally by slot management functions to clear old slot
	 * assignments before assigning new ones or when completely unequipping an ability.
	 * 
	 * @param Spec Pointer to the ability spec whose slot tag should be cleared
	 */
	void ClearSlot(FGameplayAbilitySpec* Spec);

	/**
	 * Removes the specified input slot tag from all ability specs that have it in their dynamic tags.
	 * This function iterates through all granted abilities and removes the specified Slot tag from any
	 * ability spec that contains it. Used when equipping an ability to a slot to ensure no other abilities
	 * are currently assigned to that slot, preventing multiple abilities from being bound to the same input.
	 * Called by ServerEquipAbility before assigning a new ability to a slot to maintain one-to-one slot mappings.
	 * 
	 * @param Slot The input slot tag to remove from all abilities (e.g., "InputTag.1", "InputTag.LMB")
	 */
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);

	/**
	 * Checks whether the specified ability spec has the given slot tag in its dynamic tags.
	 * Returns true if the ability spec's dynamic tags contain the specified Slot tag, false otherwise.
	 * This static utility function is used to query whether an ability is currently equipped to a specific
	 * input slot.
	 * 
	 * @param Spec Pointer to the ability spec to check
	 * @param Slot The input slot tag to search for (e.g., "InputTag.1", "InputTag.LMB")
	 * @return True if the ability spec contains the specified slot tag, false otherwise
	 */
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
	
protected:
	
	/**
	 * Replication notification callback invoked on clients when the ActivateAbilities array is replicated from the server.
	 * This override allows the component to respond to changes in the replicated abilities list,
	 * typically used to update client side state or UI when abilities are granted or removed on the server.
	 */
	virtual void OnRep_ActivateAbilities() override;
	
	// UFUNCTION(Client, Reliable) specifies this is a client RPC that must be executed on the client machine.
	// "Client" means this function will be called on the server but executed on the client
	// "Reliable" ensures the RPC call is guaranteed to arrive (TCP-like behavior vs unreliable UDP-like)
	// This function is automatically replicated from server to client when called on the server
	// The implementation function will be auto-generated as ClientEffectApplied_Implementation
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
	
	
	/**
	 * Client RPC that notifies the client about an ability status change that occurred on the server.
	 * UFUNCTION(Client, Reliable) ensures this function executes on the client when called from the server.
	 * "Reliable" guarantees the RPC will arrive even under poor network conditions.
	 * This function is called by the server after UpdateAbilityStatuses() determines that an ability's status
	 * has changed (e.g., from locked to eligible). The client then broadcasts the AbilityStatusChanged delegate
	 * to update local UI and systems with the new ability status.
	 * 
	 * Why we need a Client RPC here:
	 * The ability status changes (calculated in UpdateAbilityStatuses) occur on the server and need to be
	 * communicated to clients for UI updates. While ability specs themselves are replicated via OnRep_ActivateAbilities,
	 * the status tags within those specs are not automatically replicated in a way that triggers our delegate system.
	 * Without this RPC, clients wouldn't know when to update their UI elements (like spell menu widgets) to reflect
	 * newly unlocked or status-changed abilities. The RPC allows the server to explicitly notify clients to broadcast
	 * the AbilityStatusChanged delegate, which UI systems (like SpellMenuWidgetController) are listening to via
	 * BindCallbacksToDependencies(). This ensures the UI stays synchronized with the server's authoritative ability
	 * state changes without relying on polling or manual replication property notifications.
	 * 
	 * @param AbilityTag The gameplay tag identifying which ability's status changed
	 * @param StatusTag The new status tag for the ability (e.g., "Abilities.Status.Eligible")
	 * @param AbilityLevel The current level of the ability
	 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
};
