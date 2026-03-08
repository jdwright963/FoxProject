// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "FoxGameplayTags.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/FoxWidgetController.h"
#include "SpellMenuWidgetController.generated.h"


// Declares a dynamic multicast delegate type that can be broadcast to multiple listeners (UI widgets) when a spell
// globe is selected. FSpellGlobeSelectedSignature is the name of the delegate type  
// It passes four parameters: whether the spend points button should be enabled, whether the equip
// button should be enabled (based on the selected ability's status and available spell points), a description string
// for the current level of the ability, and a description string for the next level of the ability.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FString, DescriptionString, FString, NextLevelDescriptionString);

// Declares a dynamic multicast delegate type that is broadcast when the spell menu enters or exits "waiting for equip
// selection" mode. FWaitForEquipSelectionSignature is the name of the delegate type. It passes one parameter: the
// ability type tag (e.g., Abilities_Type_Offensive, Abilities_Type_Passive) that indicates which type of ability slot
// the player is attempting to equip an ability into, allowing UI elements to highlight valid equip target slots.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);

// Declares a dynamic multicast delegate type that is broadcast when an ability is assigned to an input slot. However, this
// delegate type has a misleading name that makes it seem like it is only broadcast when an ability is REASSIGNED from 
// one input slot to another. FSpellGlobeReassignedSignature is the name of the delegate type. It passes one parameter: 
// the ability tag that identifies which ability was assigned (e.g., Abilities_Fire_FireBolt), allowing UI elements (spell globe 
// buttons) to update their visual state if they represent this ability by deselecting the WBP_SpellGlobe_Button (removing
// the widget that is a white ring from around the spell globe button that makes it look like it is selected)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityTag);

// Struct that holds information about the currently selected ability in the spell menu UI, including the ability's
// gameplay tag and its current status (locked, unlocked, eligible, or equipped)
struct FSelectedAbility
{
	// The gameplay or ability/asset tag identifying the selected ability (e.g., Abilities_Fire_FireBolt)
	FGameplayTag Ability = FGameplayTag();
	
	// The status tag of the selected ability (e.g., Abilities_Status_Locked, Abilities_Status_Unlocked)
	FGameplayTag Status = FGameplayTag();
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class FOX_API USpellMenuWidgetController : public UFoxWidgetController
{
	GENERATED_BODY()
public:
	// Broadcasts initial values to the spell menu UI, including all ability information and the current spell points count
	virtual void BroadcastInitialValues() override;

	// Binds callback functions to dependencies: ability status changes from the ability system component and spell points
	// changes from the player state, ensuring the UI stays synchronized with game state
	virtual void BindCallbacksToDependencies() override;

	// Delegate broadcast to listening UI widgets when the player's spell points change (e.g., spending points to unlock
	// abilities or gaining points on level up)
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature SpellPointsChanged;
	
	// Delegate instance of the type defined above. It is broadcasted to listening UI widgets when a spell globe is
	// selected, communicating whether the spend points and equip buttons should be enabled based on the selected 
	// ability's status and available spell points
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;
	
	// Delegate broadcast to listening UI widgets when the spell menu enters "waiting for equip selection" mode after a 
	// WBP_SpellGlobe_Button (which is assigned an ability) is selected and the equip button is pressed. It passes the 
	// ability type tag (e.g., Abilities_Type_Offensive, Abilities_Type_Passive) to allow UI elements to highlight valid
	// input slots where the selected ability can be equipped.
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipDelegate;

	// Delegate broadcast to listening UI widgets when the spell menu exits "waiting for equip selection" mode. This occurs
	// when the player successfully equips an ability to a slot, cancels the equip operation (by selecting a different 
	// WBP_SpellGlobe_Button than they initially selected), or deselects the current ability by clicking it again. It 
	// passes the ability type tag to allow UI elements to remove highlighting from input slots.
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitingForEquipDelegate;
	
	// Delegate broadcast to listening UI widgets (specifically WBP_SpellGlobe_Button instances) when an ability is
	// assigned to an input slot. It passes the ability tag to notify spell globe buttons
	// representing that ability to update their visual state (typically deselecting/removing the white ring indicator).
	// Despite its name suggesting reassignment, this delegate is broadcast for any ability-to-slot assignment operation.
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;
	
	// This function is called in WBP_SpellGlobe_Button when a spell globe is selected, and the Ability Tag of the
	// spell globe that was selected is passed into this function. It retrieves the ability's status and determines
	// which buttons should be enabled, then broadcasts this information via SpellGlobeSelectedDelegate to update the UI.
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);
	
	// Called when the spend points button is pressed in the spell menu UI. This function spends a spell point to upgrade
	// the currently selected ability (stored in SelectedAbility), progressing it through the status chain: Locked ->
	// Eligible -> Unlocked -> Equipped. It communicates with the ability system component to update the ability status
	// and triggers UI updates to reflect the new ability state and remaining spell points.
	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();
	
	// Called in WBP_SpellMenu when a spell globe is deselected in the spell menu UI (e.g., clicking the same globe again).
	// This function resets the selected ability back to the default "None" ability with "Locked"
	// status and broadcasts the deselected state to the UI via SpellGlobeSelectedDelegate, disabling both the spend
	// points and equip buttons and clearing the ability description displays.
	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();
	
	// Called when the equip button is pressed in the spell menu UI (WBP_SpellMenu). This function implements "waiting for 
	// equip selection" mode where we wait for the player to select an input slot to assign the currently selected 
	// ability to. It broadcasts the WaitForEquipDelegate with the selected ability's type tag, prompting the UI to 
	// highlight valid equip slots (e.g., active input slots for offensive abilities, or passive slots for passive 
	// abilities). The equip operation completes when the player clicks a slot.
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();
	
	// Called when an input slot globe (WBP_EquippedRow_Button in the input row, e.g., a passive slot or an offensive ability
	// slot) is pressed while in "waiting for equip selection" mode in WBP_SpellMenu. This function equips the currently selected ability 
	// (stored in SelectedAbility) to the clicked slot, exits "waiting for equip selection" mode by broadcasting 
	// StopWaitingForEquipDelegate, and updates the ability system component with the new slot assignment. The SlotTag 
	// parameter identifies which input slot was clicked (e.g., InputTag_1, InputTag_2, InputTag_Passive_1), and the 
	// AbilityType parameter indicates the type of slot (e.g., Abilities_Type_Offensive, Abilities_Type_Passive).
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);

	// Callback function bound to the ability system component's OnAbilityEquipped delegate. This function is triggered
	// whenever an ability is equipped or re-equipped to a different slot, through the spell menu UI. It updates the UI 
	// to reflect the new ability-to-slot assignments by broadcasting the updated ability info
	// through OnAbilityInfoDelegate. The parameters indicate which ability was equipped (AbilityTag), its status after
	// equipping (Status), the slot it was equipped to (Slot), and the slot it was previously in if any (PreviousSlot).
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);

private:
	
	// Function that determines which buttons should be enabled in the spell menu UI based on the ability's current
	// status tag and available spell points. The last two parameters (bShouldEnableSpellPointsButton and 
	// bShouldEnableEquipButton) are passed by reference so they can be set by this function as output parameters that
	// indicate whether the spend points and equip buttons should be enabled.
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);
	
	// Stores the currently selected ability in the spell menu UI, initialized to "None" ability with "Locked" status
	// as the default state before any spell globe is selected by the player. Updated when SpellGlobeSelected() is called.
	FSelectedAbility SelectedAbility = { FFoxGameplayTags::Get().Abilities_None,  FFoxGameplayTags::Get().Abilities_Status_Locked };
	
	// Cached value of the player's current spell points, updated when spell points change and used to determine button states
	int32 CurrentSpellPoints = 0;
	
	// Boolean flag that tracks whether the spell menu is currently in "waiting for equip selection" mode, where the player
	// is expected to click an input slot to assign the selected ability. Set to true when EquipButtonPressed() is called,
	// and set to false when an ability is equipped, the selection is cancelled, or the ability is deselected.
	bool bWaitingForEquipSelection = false;
	
	// Stores the gameplay tag of the input slot that was clicked during "waiting for equip selection" mode, used to track
	// which slot the player wants to assign the currently selected ability to (e.g., InputTag_1, InputTag_2, InputTag_Passive_1).
	FGameplayTag SelectedSlot;
};
