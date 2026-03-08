// Copyright TryingToMakeGames


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "FoxGameplayTags.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/FoxPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	// Broadcast all ability information to the widget when the controller is initialized
	// this function is inherited by this class
	BroadcastAbilityInfo();
	
	// Broadcast the current spell points value from the PlayerState to all listening UI widgets (e.g., spell menu).
	// This ensures the UI displays the correct initial spell points count when the widget controller is first initialized.
	SpellPointsChanged.Broadcast(GetFoxPS()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	// Bind a lambda function to the AbilityStatusChanged delegate in FoxAbilitySystemComponent.h to receive notifications
	// whenever an ability's status changes (e.g., from Locked to Unlocked, or from Unequipped to Equipped). This 
	// ensures the spell menu UI stays synchronized with the underlying ability system state
	// Lambda parameters: AbilityTag identifies which ability changed (e.g., "Abilities.Fire"), StatusTag contains
	// the new status value (e.g., "Abilities.Status.Unlocked"), NewLevel contains the updated level of the ability
	// after the status change
	// [this] captures the USpellMenuWidgetController instance pointer, allowing the lambda to access member variables
	// (e.g., AbilityInfo, AbilityInfoDelegate) and member functions of this widget controller
	GetFoxASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		// Check if the ability that just changed status is the currently selected ability in the spell menu UI by
		// comparing its tag exactly with the SelectedAbility struct's Ability tag.
		if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
		{
			// Update the status tag in the SelectedAbility struct to reflect the new status (the StatusTag input parameter
			// of this lambda callback) of the currently selected ability. This keeps the cached selected ability data 
			// synchronized with the ability system's actual state
			SelectedAbility.Status = StatusTag;

			// Initialize boolean flags to track whether the Spend Points and Equip buttons should be enabled based on
			// the selected ability's new status and the player's current spell points. These will be set by ShouldEnableButtons
			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;

			// Call the helper function to evaluate the ability's new status and current spell points to determine whether
			// each button should be enabled. The function updates bEnableSpendPoints and bEnableEquip by reference based
			// on game rules (e.g., if the ability just became Eligible and we have spell points, enable Spend Points button)
			//
			// So far our understanding is that if this delegate callback gets called before the callback for 
			// OnSpellPointsChangedDelegate then this function call here will pass the incorrect value for 
			// CurrentSpellPoints, but that is ok because when the OnSpellPointsChangedDelegate lambda callback calls 
			// this function (ShouldEnableButtons) again it will pass the correct value for CurrentSpellPoints. The same 
			// is true if the other delegate callback gets called first. In that case, we will first call this function
			// with the incorrect value for StatusTag, but then when this lambda callback calls this function again it 
			// will pass the correct value for StatusTag. So, after the second call to this function we will have both
			// of the correct values for StatusTag and CurrentSpellPoints.
			ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
			
			// Declare a string variable to store the description text for the current level of the selected ability.
			// This will be populated by GetDescriptionsByAbilityTag() and broadcast to the UI to display ability details.
			FString Description;

			// Declare a string variable to store the description text for the next level of the selected ability. This
			// will be populated by GetDescriptionsByAbilityTag() and broadcast to the UI to show what benefits the player
			// would gain by leveling up the ability.
			FString NextLevelDescription;

			// Use the getter function for the ASC that this class inherits. Then, Call the ability system component's
			// helper function to retrieve the formatted description strings for both
			// the current level and next level of the ability identified by AbilityTag. The Description and 
			// NextLevelDescription variables are passed by reference and will be populated with the appropriate text
			// based on the ability's current level and status.
			GetFoxASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);

			// Broadcast the updated button enable states and description strings to all listening UI widgets so they can
			// refresh their visual state to reflect the status change of the currently selected ability (e.g., enable the
			// Equip button if the ability just became Unlocked, and display updated ability descriptions for the current
			// and next levels).
			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		}
		
		// Verify that the AbilityInfo data asset is valid before attempting to query it. This prevents crashes if the
		// data asset reference hasn't been set or was cleared.
		//
		// AbilityInfo is a protected member variable (TObjectPtr<UAbilityInfo>) inherited from the parent class
		// UFoxWidgetController. It points to a UAbilityInfo data asset that contains configuration data for all
		// abilities in the game.
		if (AbilityInfo)
		{
			// Retrieve the FFoxAbilityInfo struct for the changed ability from the data asset. This gives
			// us all the display data (icon, background, etc.)
			FFoxAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			
			// Update the StatusTag field of the ability info struct with the new status received from the delegate. 
			// This creates a complete, up-to-date ability info struct that includes both the static configuration data
			// and the dynamic status
			Info.StatusTag = StatusTag;
			
			// Broadcast the updated ability information to all listening UI widgets (spell menu buttons) so
			// they can refresh their visual state to reflect the status change (e.g., change appearance from Locked to 
			// Eligible)
			AbilityInfoDelegate.Broadcast(Info);
		}
	});
	
	// Bind the OnAbilityEquipped member function to the AbilityEquipped delegate in FoxAbilitySystemComponent so this function
	// is called whenever (this delegate broadcasts) an ability is successfully equipped to an input slot (or 
	// unequipped/moved to a different slot).
	//
	// This ensures the spell menu UI stays synchronized with the ability system's equipped state by updating the visual
	// representation of input slots and ability status tags when equip operations complete on the server.
	// The OnAbilityEquipped callback receives four parameters: AbilityTag (identifies which ability was equipped),
	// Status (the new status tag, typically "Abilities.Status.Equipped"), Slot (the input tag the ability was equipped to),
	// and PreviousSlot (the input tag the ability was previously equipped to, if any, used to clear old slot state).
	GetFoxASC()->AbilityEquipped.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);
	
	// Bind a lambda function to the OnSpellPointsChangedDelegate in the FoxPlayerState to receive notifications whenever
	// the player's spell points change (e.g., when spending points to unlock abilities or gaining points on level up).
	// This ensures the spell menu UI stays synchronized with the current spell points count stored in PlayerState.
	// Lambda parameter: SpellPoints contains the new spell points value after the change.
	// [this] captures the USpellMenuWidgetController instance pointer, allowing the lambda to access the SpellPointsChanged
	// delegate member variable to broadcast updates to UI widgets.
	GetFoxPS()->OnSpellPointsChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		// Broadcast the updated spell points value to all listening UI widgets (e.g., spell menu display) so they can
		// refresh their visual state to show the current available spell points
		// This is a different delegate than the one that called this lambda callback function
		SpellPointsChanged.Broadcast(SpellPoints);
		
		// Update the cached CurrentSpellPoints member variable with the new spell points value (the SpellPoints input
		// parameter of this lambda callback).
		CurrentSpellPoints = SpellPoints;

		// Initialize boolean flags to track whether the Spend Points and Equip buttons should be enabled based on the
		// currently selected ability's status and the new spell points value. These will be set by ShouldEnableButtons
		bool bEnableSpendPoints = false;
		bool bEnableEquip = false;
		
		// Call the helper function to evaluate the currently selected ability's status and the new spell points count
		// to determine whether each button should be enabled. The function updates bEnableSpendPoints and bEnableEquip
		// by reference based on game rules (e.g., enable Spend Points button if spell points increased from 0 to 1)
		ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
		
		// Declare a string variable to store the description text for the current level of the currently selected ability.
		// This will be populated by GetDescriptionsByAbilityTag() and broadcast to the UI to display ability details.
		FString Description;

		// Declare a string variable to store the description text for the next level of the currently selected ability.
		// This will be populated by GetDescriptionsByAbilityTag() and broadcast to the UI to show what benefits the player
		// would gain by leveling up the ability.
		FString NextLevelDescription;

		// Use the getter function for the ASC that this class inherits. Then, call the ability system component's helper
		// function to retrieve the formatted description strings for both the current level and next level of the currently
		// selected ability (stored in SelectedAbility.Ability). The Description and NextLevelDescription variables are
		// passed by reference and will be populated with the appropriate text based on the ability's current level and status.
		GetFoxASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);

		// Broadcast the updated button enable states and description strings to all listening UI widgets so they can
		// refresh their visual state to reflect the spell points change (e.g., enable the Spend Points button if spell
		// points just became available, and display updated ability descriptions for the current and next levels).
		SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	// Check if the widget controller is currently in a state where it's waiting for the player to select an input slot
	// to equip an ability to (this flag is set to true when EquipButtonPressed() is called). If true, this means the
	// player has now selected a different spell globe while in equip mode, so we need to cancel the equip operation
	if (bWaitingForEquipSelection)
	{
		// Retrieve the ability type tag (e.g., "Abilities.Type.Offensive", "Abilities.Type.Passive") of the newly
		// selected ability from the AbilityInfo data asset. This type information is needed to properly clear UI state
		// for the specific category of input slots that were highlighted during the equip operation
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;

		// Broadcast the stop waiting delegate with the ability type to notify all listening UI widgets (spell menu buttons)
		// to exit equip selection mode and restore normal visual state (e.g., un-highlight input slot buttons that were
		// highlighted for the previous ability type). This cancels the equip operation that was initiated by the previous
		// EquipButtonPressed() call
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);

		// Reset the waiting flag to false to indicate that the widget controller is no longer in equip selection mode.
		bWaitingForEquipSelection = false;
	}
	
	// Retrieve the singleton instance of FFoxGameplayTags which provides access to all gameplay tags used in the
	// Fox project (e.g., Abilities_None, Abilities_Status_Locked). Using the singleton avoids repeated tag lookups and
	// ensures consistent tag references throughout the codebase
	const FFoxGameplayTags GameplayTags = FFoxGameplayTags::Get();

	// Retrieve the current spell points count from the FoxPlayerState. This value is needed to determine whether the
	// Spend Points button should be enabled (requires SpellPoints > 0)
	const int32 SpellPoints = GetFoxPS()->GetSpellPoints();

	// Declare a FGameplayTag variable to store the ability's current status (e.g., Locked, Eligible, Unlocked, Equipped).
	// This will be populated based on validation checks and the ability spec's status tag
	FGameplayTag AbilityStatus;	

	// Perform validation checks on the selected ability tag to ensure it's valid and refers to an actual ability.
	// bTagValid checks if the tag is properly initialized, bTagNone checks if it matches the "Abilities.None" tag
	// (indicating no ability selected)
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);

	// Retrieve the FGameplayAbilitySpec from the ability system component for the ability tag passed in. The spec contains
	// runtime data about the ability (level, status, etc.). bSpecValid checks if the spec exists (nullptr indicates the
	// ability hasn't been granted to this player)
	const FGameplayAbilitySpec* AbilitySpec = GetFoxASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	
	// If the ability tag is invalid, matches "Abilities.None", or the ability spec doesn't exist, treat the ability as
	// Locked by setting the AbilityStatus variable to the Locked tag. This handles edge cases.
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}
	// Otherwise, get the ASC and retrieve the actual status from the valid ability spec (e.g., Eligible, Unlocked, Equipped) using the
	// ability system component's helper function
	else
	{
		AbilityStatus = GetFoxASC()->GetStatusFromSpec(*AbilitySpec);
	}
	
	// Store the ability tag of the currently selected spell globe in the SelectedAbility struct member variable. This
	// tracks which ability the player has selected in the spell menu UI
	SelectedAbility.Ability = AbilityTag;

	// Store the determined status tag (Locked, Eligible, Unlocked, or Equipped) of the currently selected ability in
	// the SelectedAbility struct member variable.
	SelectedAbility.Status = AbilityStatus;
	
	// Initialize boolean flags to track whether the Spend Points and Equip buttons should be enabled in the spell menu UI.
	// These flags will be set by the ShouldEnableButtons helper function based on the ability's current status and available
	// spell points
	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;

	// Call the helper function to evaluate the ability's status and available spell points to determine whether each button
	// should be enabled. The function updates bEnableSpendPoints and bEnableEquip by reference based on game rules (e.g.,
	// Spend Points button is enabled if the ability is Eligible/Unlocked/Equipped and spell points > 0)
	ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpendPoints, bEnableEquip);

	// Declare a string variable to store the description text for the current level of the selected ability. This will be
	// populated by GetDescriptionsByAbilityTag() and broadcast to the UI to display ability details.
	FString Description;

	// Declare a string variable to store the description text for the next level of the selected ability. This will be
	// populated by GetDescriptionsByAbilityTag() and broadcast to the UI to show what benefits the player would gain by
	// leveling up the ability.
	FString NextLevelDescription;

	// Use the getter function for the ASC that this class inherits. Then, call the ability system component's helper
	// function to retrieve the formatted description strings for both the current level and next level of the ability
	// identified by AbilityTag. The Description and NextLevelDescription variables are passed by reference and will be
	// populated with the appropriate text based on the ability's current level and status.
	GetFoxASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);

	// Broadcast the button enable states and description strings to all listening UI widgets (spell menu buttons) so they
	// can update their visual state (enabled/disabled) based on the selected spell globe's status and the player's available
	// spell points, and display the ability descriptions for the current and next levels.
	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	// Use the inherited ASC getter function.
	// Verify that the Fox ability system component is valid before attempting to spend a spell point. This prevents
	// crashes if the component reference is null or invalid
	if (GetFoxASC())
	{
		// Call the server RPC function to spend a spell point on the currently selected ability (stored in 
		// SelectedAbility.Ability). This sends a request to the server to unlock or level up the ability, ensuring
		// authoritative validation of the spell point transaction
		GetFoxASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::GlobeDeselect()
{
	// Check if the widget controller is currently in a state where it's waiting for the player to select an input slot
	// to equip an ability to (this flag is set to true when EquipButtonPressed() is called). If true, this means the
	// player has deselected (this function is only called when an already selected spell globe is clicked again) the 
	// spell globe while in equip mode, so we need to cancel the equip operation
	if (bWaitingForEquipSelection)
	{
		// Retrieve the ability type tag (e.g., "Abilities.Type.Offensive", "Abilities.Type.Passive") of the currently
		// selected ability (stored in SelectedAbility.Ability) from the AbilityInfo data asset. This type information
		// is needed to properly clear UI state for the specific category of input slots that were highlighted during
		// the equip operation
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

		// Broadcast the stop waiting delegate with the ability type to notify all listening UI widgets (spell menu buttons)
		// to exit equip selection mode and restore normal visual state (e.g., un-highlight input slot buttons that were
		// highlighted for this ability type). This cancels the equip operation that was initiated by EquipButtonPressed()
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);

		// Reset the waiting flag to false to indicate that the widget controller is no longer in equip selection mode.
		// This clears the state set by EquipButtonPressed()
		bWaitingForEquipSelection = false;
	}
	
	// Reset the cached selected ability tag to "Abilities.None" to indicate that no ability is currently selected in
	// the spell menu UI. This clears the previous selection when the player deselects a spell globe
	SelectedAbility.Ability = FFoxGameplayTags::Get().Abilities_None;

	// Reset the cached selected ability status tag to "Abilities.Status.Locked" to clear the previous ability's status
	// information from the SelectedAbility struct. This ensures the cached data doesn't contain stale status information
	// when no ability is selected
	SelectedAbility.Status = FFoxGameplayTags::Get().Abilities_Status_Locked;

	// Broadcast the deselection state to all listening UI widgets with both buttons disabled (false, false) and empty
	// description strings (FString(), FString()) to update the spell menu UI to reflect that no ability is currently
	// selected (e.g., show no ability descriptions, and disable interaction buttons)
	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	// Retrieve the ability type tag (e.g., "Abilities.Type.Offensive", "Abilities.Type.Passive") of the currently
	// selected ability (stored in SelectedAbility.Ability) from the AbilityInfo data asset. This type information is
	// needed to determine which category of input slots should be highlighted in the spell menu UI during equip
	// selection mode (e.g., offensive abilities can be equipped to slots 1-4, LMB, RMB)
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	// Broadcast the wait for equip delegate with the ability type to notify all listening UI widgets (spell menu buttons)
	// to enter equip selection mode and highlight the appropriate input slot buttons for this ability type. This signals
	// the UI to highlight valid equip slots (e.g., active input slots for offensive abilities, or passive slots for 
	// passive abilities). The equip operation completes when the player clicks a slot.
	WaitForEquipDelegate.Broadcast(AbilityType);

	// Set the waiting flag to true to indicate that the widget controller is now in equip selection mode, waiting for
	// the player to click an input slot button. This flag is checked in SpellGlobeSelected() and GlobeDeselect() to
	// handle cancellation of the equip operation if the player selects or deselects a different spell globe
	bWaitingForEquipSelection = true;
	
	// Retrieve the current status tag (e.g., Locked, Eligible, Unlocked, Equipped) of the currently selected ability
	// from the ability system component. This is needed to determine if the ability is already equipped to an input
	// slot, which affects how the equip operation should be handled (e.g., moving an equipped ability to a different slot)
	const FGameplayTag SelectedStatus = GetFoxASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);

	// Check if the selected ability's status exactly matches "Abilities.Status.Equipped", meaning the ability is currently
	// equipped to an input slot. If true, we need to retrieve which slot it's equipped to so the equip operation can
	// properly handle moving the ability from its current slot to the new slot the player selects
	if (SelectedStatus.MatchesTagExact(FFoxGameplayTags::Get().Abilities_Status_Equipped))
	{
		// Retrieve the input tag (e.g., "InputTag.1", "InputTag.LMB") of the slot that the selected ability is currently
		// equipped to, and store it in the SelectedSlot member variable. This information is needed by the server RPC
		// (ServerEquipAbility) to properly unequip the ability from its current slot before equipping it to the new slot
		SelectedSlot = GetFoxASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	// Check if the widget controller is currently in equip selection mode (waiting for the player to click an input slot).
	// If false, the player clicked an input slot button without first pressing the Equip button, so ignore this click
	// and return early to prevent invalid equip operations
	if (!bWaitingForEquipSelection) return;

	// Retrieve the ability type tag (e.g., "Abilities.Type.Offensive", "Abilities.Type.Passive") of the currently selected
	// ability (stored in SelectedAbility.Ability) from the AbilityInfo data asset. This is needed to validate that the
	// player clicked a slot that matches the selected ability's type (e.g., can't equip a passive ability to an active slot)
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	// Verify that the ability type of the clicked slot (AbilityType parameter) exactly matches the selected ability's type.
	// If they don't match, the player clicked an invalid slot for this ability type (e.g., clicked a passive slot while
	// trying to equip an offensive ability), so return early to prevent the invalid equip operation
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	// Call the server RPC function to equip the currently selected ability (SelectedAbility.Ability) to the clicked input
	// slot (SlotTag). This sends a request to the server to perform the equip operation, ensuring authoritative validation
	// of the equip transaction and proper replication to all clients
	GetFoxASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	// Reset the waiting flag to false to indicate that the equip operation has completed and the widget controller is
	// no longer in equip selection mode (the mode that was activated by EquipButtonPressed()).
	bWaitingForEquipSelection = false;

	// Retrieve the singleton instance of FFoxGameplayTags which provides access to all gameplay tags used in the Fox
	// project (e.g., Abilities_None, Abilities_Status_Unlocked). Using the singleton avoids repeated tag lookups.
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	// Declare and initialize an FFoxAbilityInfo struct to represent the cleared state of the input slot that the ability
	// was previously equipped to (PreviousSlot). This struct will be populated with "empty" values and broadcast to update
	// the UI to show that the previous slot no longer has an ability equipped to it.
	FFoxAbilityInfo LastSlotInfo;
	
	// Set the status tag to "Abilities.Status.Unlocked" because in this UI system, input slot buttons (e.g., the "1", "2",
	// "LMB", "RMB" buttons on the spell bar) subscribe to the AbilityInfoDelegate and use FFoxAbilityInfo structs to determine
	// their visual appearance. When an ability is moved/unequipped from a slot, we broadcast an FFoxAbilityInfo with
	// StatusTag=Unlocked to tell that slot's button "you're now empty but available for new abilities" (as opposed to
	// Locked which would show as disabled/unusable). This reuses the same ability status tags for slot state management.
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;

	// Set the input tag to PreviousSlot so when we broadcast this LastSlotInfo struct via AbilityInfoDelegate, the UI
	// widget listening to that delegate knows which specific input slot button (identified by its InputTag like "InputTag.1")
	// needs to update its visual state. Input slot buttons filter the broadcast FFoxAbilityInfo structs by matching their
	// own InputTag against the struct's InputTag field to determine if the update applies to them. This is how we target
	// the specific slot that needs to be cleared.
	LastSlotInfo.InputTag = PreviousSlot;

	// Set the ability tag to "Abilities.None" because input slot buttons display which ability (if any) is currently equipped
	// to them by reading the AbilityTag field of the FFoxAbilityInfo struct they receive via AbilityInfoDelegate. Setting
	// this to Abilities_None tells the slot button "no ability is equipped to you anymore", which triggers it to clear its
	// ability icon and show an empty/default appearance. This completes the "empty slot" data that will be broadcast to
	// clear the previous slot's UI representation.
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;

	// Broadcast the cleared last slot info to all listening UI widgets (spell menu buttons) to update the visual state
	// of the input slot that the ability was previously equipped to, showing it as empty and available for equipping
	// other abilities
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// Retrieve the full FFoxAbilityInfo struct for the ability that was just equipped from the AbilityInfo data asset.
	// This gives us all the display data (icon, background, name, description, etc.) for the newly equipped ability
	FFoxAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

	// Update the StatusTag field of the ability info struct with the new status (typically "Abilities.Status.Equipped")
	// received from the delegate. This ensures the struct contains the current runtime status of the ability
	Info.StatusTag = Status;

	// Update the InputTag field of the ability info struct with the new slot tag (e.g., "InputTag.1", "InputTag.LMB")
	// that the ability was equipped to. This links the ability data to its equipped slot for UI display purposes
	Info.InputTag = Slot;

	// Broadcast the updated ability info to all listening UI widgets (spell menu buttons and input slot buttons) to
	// refresh their visual state to reflect the completed equip operation (e.g., show the ability icon in the new slot,
	// update status indicators)
	AbilityInfoDelegate.Broadcast(Info);

	// Broadcast the stop waiting delegate with the equipped ability's type (e.g., "Abilities.Type.Offensive") to notify
	// all listening UI widgets to exit equip selection mode and restore normal visual state (e.g., un-highlight the input
	// slot buttons that were highlighted during the equip operation). This completes the equip operation initiated by
	// EquipButtonPressed()
	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	
	// Broadcast the spell globe reassigned delegate with the ability tag to notify listening UI widgets (spell globe
	// buttons) that this ability has been equipped to a slot, so they can update their visual state if they represent 
	// this ability by deselecting the WBP_SpellGlobe_Button (removing the widget that is a white ring from around the 
	// spell globe button that makes it look like it is selected)
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	
	// Call GlobeDeselect() to clear the selected ability state and update the spell menu UI to reflect that no ability
	// is currently selected (not by deselecting the spell globe like the line above, but by disabling the equip and spend
	// points button and showing empty description boxes). This resets the selection UI after the equip operation completes,
	// returning the spell menu to its default state with disabled buttons and cleared description panels
	GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	// Retrieve the singleton instance of FFoxGameplayTags to access all gameplay tags used in the Fox project
	const FFoxGameplayTags GameplayTags = FFoxGameplayTags::Get();

	// Initialize both button enable flags to false by default. They will only be set to true if the ability status and
	// spell points meet specific conditions in the subsequent if-else chain, ensuring buttons are disabled unless
	// explicitly enabled by game rules
	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;

	// Check if the ability status exactly matches "Abilities.Status.Equipped", meaning the ability is currently equipped
	// to an input slot.
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		// Enable the Equip button for equipped abilities to allow the player to unequip the ability from its current
		// input slot. The button label/functionality will change to "Unequip" in the UI when an equipped ability is selected
		bShouldEnableEquipButton = true;

		// Check if the player has at least one spell point available to spend. Spell points are required to level up
		// abilities, so the Spend Points button should only be enabled if the player has points to spend
		if (SpellPoints > 0)
		{
			// Enable the Spend Points button for equipped abilities when spell points are available, allowing the player
			// to spend points to increase the ability's level (e.g., increase damage, reduce cooldown). This follows the
			// game rule that abilities can be leveled up while equipped
			bShouldEnableSpellPointsButton = true;
		}
	}
	// Check if the ability status exactly matches "Abilities.Status.Eligible", meaning the ability is available to be
	// unlocked but hasn't been unlocked yet (e.g., player level requirement met).
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		// Check if the player has at least one spell point available to spend. Eligible abilities require spending a
		// spell point to unlock them for the first time
		if (SpellPoints > 0)
		{
			// Enable the Spend Points button for eligible abilities when spell points are available, allowing the player
			// to spend a point to unlock the ability. The Equip button remains disabled because the ability must be
			// unlocked before it can be equipped to an input slot
			bShouldEnableSpellPointsButton = true;
		}
	}
	// Check if the ability status exactly matches "Abilities.Status.Unlocked", meaning the ability has been unlocked
	// (purchased with a spell point) but is not currently equipped to any input slot.
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		// Enable the Equip button for unlocked abilities to allow the player to equip the ability to an available input
		// slot, making it usable in gameplay
		bShouldEnableEquipButton = true;
		
		// Check if the player has at least one spell point available to spend. Unlocked abilities can be leveled up by
		// spending additional spell points to increase their effectiveness
		if (SpellPoints > 0)
		{
			// Enable the Spend Points button for unlocked abilities when spell points are available, allowing the player
			// to spend points to increase the ability's level even before equipping it. This follows the game rule that
			// abilities can be leveled up while in the unlocked (but unequipped) state
			bShouldEnableSpellPointsButton = true;
		}
	}
}
