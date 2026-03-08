// Copyright TryingToMakeGames

#include "UI/WidgetController/OverlayWidgetController.h"

#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/FoxPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	/*
	 * Use the getter function that this class inherits to get the FoxAttributeSet.
	 * Broadcast the initial Health value to any bound UI widgets.
	 * This is a custom delegate broadcast (not the GAS automatic delegate) that manually sends the current
	 * Health value on initialization. Unlike GAS's built-in delegates which automatically broadcast when an
	 * attribute changes, our custom delegates (OnHealthChanged, OnMaxHealthChanged, etc.) require manual
	 * broadcasting to ensure UI widgets display the correct initial values when first created.
	 */
	OnHealthChanged.Broadcast(GetFoxAS()->GetHealth());

	// Broadcast the initial MaxHealth value to any bound UI widgets. See OnHealthChanged comment for details.
	OnMaxHealthChanged.Broadcast(GetFoxAS()->GetMaxHealth());

	// Broadcast the initial Mana value to any bound UI widgets. See OnHealthChanged comment for details.
	OnManaChanged.Broadcast(GetFoxAS()->GetMana());

	// Broadcast the initial MaxMana value to any bound UI widgets. See OnHealthChanged comment for details.
	OnMaxManaChanged.Broadcast(GetFoxAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	/*
	 * Use the getter function that this class inherits to get the FoxPlayerState.
	 * Bind our OnXPChanged member function to the FoxPlayerState's OnXPChangedDelegate multicast delegate.
	 * 
	 * AddUObject is a delegate binding function that binds a UObject-derived class member function to a delegate.
	 * Parameters:
	 *   - 'this': The UOverlayWidgetController instance that owns the OnXPChanged callback function
	 *   - '&UOverlayWidgetController::OnXPChanged': Pointer to the member function to be called
	 * 
	 * When AFoxPlayerState's XP changes (via AddToXP() or SetXP()), the PlayerState broadcasts the new XP value
	 * through OnXPChangedDelegate, which automatically invokes our OnXPChanged callback function with the new
	 * XP value as a parameter. This allows our widget controller to react to XP changes and update the UI
	 * (specifically the XP bar progress) accordingly.
	 */
	GetFoxPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	
	/*
	 * Use the getter function that this class inherits to get the FoxPlayerState.
	 * Bind a lambda callback to the OnLevelChangedDelegate multicast delegate declared in FoxPlayerState.h
	 * 
	 * AddLambda binds an inline lambda function to a delegate, providing a lightweight alternative
	 * to creating a separate named member function when the callback logic is simple.
	 * 
	 * Lambda capture list [this]:
	 *   - Captures the current UOverlayWidgetController instance, allowing access to member variables
	 *	 like OnPlayerLevelChangedDelegate inside the lambda
	 * 
	 * Lambda parameter (int32 NewLevel):
	 *   - When AFoxPlayerState's level changes (via AddToPlayerLevel() or LevelUp()), the PlayerState
	 *	 broadcasts the new level value through OnLevelChangedDelegate, which automatically invokes
	 *	 this lambda with the new level as a parameter
	 * 
	 * Lambda body:
	 *   - Simply forwards the NewLevel value to a delegate declared in this class (OverlayWidgetController.h)
	 *   OnPlayerLevelChangedDelegate, which UI widgets (such as level display widgets) bind to in order to receive 
	 *   level updates
	 *   - This acts as a pass-through/relay: PlayerState broadcasts level changes, we receive them
	 *	 via this lambda, and immediately re-broadcast them to our UI widgets
	 * 
	 * This pattern allows the widget controller to act as an intermediary layer between game logic
	 * (PlayerState) and UI (widgets), decoupling them and allowing the widget controller to
	 * potentially add additional logic or filtering before broadcasting to UI if needed in the future.
	 */
	GetFoxPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel);
		}
	);
	
	/*
	 * Bind a lambda callback to the GAS attribute value change delegate for the Health attribute.
	 * 
	 * GetGameplayAttributeValueChangeDelegate() returns a multicast delegate that GAS automatically broadcasts
	 * whenever the specified attribute (Health) changes through gameplay effects, abilities, or direct modification.
	 * 
	 * The parameter passed to GetGameplayAttributeValueChangeDelegate():
	 * - GetFoxAS() is the getter function inherited by this class (OverlayWidgetController.cpp) it returns the player's
	 *   ability system component.
	 * - GetFoxAS()->GetHealthAttribute() returns a FGameplayAttribute struct that acts as a "handle" or "identifier"
	 *   for the Health attribute within the GAS system
	 * - FGameplayAttribute is essentially metadata that tells GAS which specific attribute property we want to monitor
	 * - GetHealthAttribute() is a static helper function (defined via ATTRIBUTE_ACCESSORS macro in FoxAttributeSet.h)
	 *   that returns: FGameplayAttribute(UFoxAttributeSet::GetHealthAttribute())
	 * - This FGameplayAttribute struct contains a pointer to the attribute's UProperty and the attribute set class,
	 *   allowing GAS to locate and monitor the specific Health float property in memory
	 * - Think of it like passing a "subscription key" that tells the delegate system "notify me when THIS specific
	 *   attribute changes", rather than monitoring all attributes or requiring string-based lookups
	 * 
	 * AddLambda binds an inline lambda function to this delegate, providing a lightweight way to handle
	 * attribute changes without creating a separate named member function.
	 * 
	 * Lambda capture list [this]:
	 *   - Captures the current UOverlayWidgetController instance, allowing access to member variables
	 *     like OnHealthChanged inside the lambda
	 * 
	 * Lambda parameter (const FOnAttributeChangeData& Data):
	 *   - When Health changes, GAS automatically invokes this lambda with attribute change information
	 *   - FOnAttributeChangeData struct contains:
	 *       * Attribute: The FGameplayAttribute that changed (Health)
	 *       * OldValue: The previous health value before the change
	 *       * NewValue: The current health value after the change
	 * 
	 * Lambda body:
	 *   - Extracts the NewValue from the Data struct and broadcasts it through OnHealthChanged delegate
	 *   - This forwards the attribute change from GAS to UI widgets that bind to OnHealthChanged
	 *   - Acts as a relay: GAS broadcasts attribute changes, we receive them via this lambda,
	 *     and immediately re-broadcast them to UI widgets for display updates
	 */
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetFoxAS()->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}	
		);
	
	// Binds a callback to broadcast MaxHealth changes when the MaxHealth attribute changes. See Health comments above for details.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetFoxAS()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}	
	);
	
	// Binds a callback to broadcast Mana changes when the Mana attribute changes. See Health comments above for details.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetFoxAS()->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}	
	);
	
	// Binds a callback to broadcast MaxMana changes when the MaxMana attribute changes. See Health comments above for details.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetFoxAS()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}	
	);
	
	/*
	* Use the getter function that this class inherits to get the FoxAbilitySystemComponent and check if it's valid
	* 
	* This entire block handles two important initialization scenarios:
	* 1. Setting up ability information broadcasting when startup abilities have been granted
	* 2. Setting up effect asset tag broadcasting for UI message widgets (like damage numbers, potion pickups, etc.)
	*/
	if (GetFoxASC())
	{
		/*
		 * Use the getter function that this class inherits to get the FoxAbilitySystemComponent.
		 * Then, bind our OnAbilityEquipped member function to the AbilityEquipped multicast delegate.
		 * 
		 * AddUObject is a delegate binding function that binds a UObject-derived class member function to a delegate.
		 * Parameters:
		 *   - 'this': The UOverlayWidgetController instance that owns the OnAbilityEquipped callback function
		 *   - '&UOverlayWidgetController::OnAbilityEquipped': Pointer to the member function to be called
		 * 
		 * When an ability is equipped to an input slot (e.g., via the spell menu), the AbilitySystemComponent broadcasts
		 * through AbilityEquipped delegate with information about which ability was equipped, its status, the slot it was
		 * equipped to, and the previous slot it occupied (if any). This automatically invokes our OnAbilityEquipped callback
		 * function with these parameters, allowing the widget controller to update the UI accordingly by broadcasting the
		 * ability info changes to listening widgets (such as spell bar slot buttons and spell menu buttons).
		 */
		GetFoxASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		
		/*
		 * Check if the startup abilities have already been granted to this ASC.
		 * bStartupAbilitiesGiven is a boolean flag in UFoxAbilitySystemComponent that gets set to true
		 * in AddCharacterAbilities() after all startup abilities have been granted.
		 * 
		 * This check handles the timing issue where the widget controller might be initialized AFTER
		 * abilities have already been granted (common scenario when UI is created after the character).
		 */
		if (GetFoxASC()->bStartupAbilitiesGiven)
		{
			/*
			 * Abilities have already been granted, so immediately broadcast ability information to the UI.
			 * 
			 * BroadcastAbilityInfo() is a function that retrieves information about all granted abilities
			 * (from an AbilityInfo data asset) and broadcasts it through a delegate that UI widgets
			 * listen to. This allows ability-related UI elements (such as ability icons, cooldowns, costs)
			 * to populate themselves with the correct data.
			 * 
			 * This immediate broadcast handles the timing scenario where the widget controller is created
			 * AFTER the character's abilities have already been granted (common when UI is initialized
			 * after character spawning). Since the abilities are already available, we don't need to wait
			 * for the AbilitiesGivenDelegate, and we can send ability data to the UI right away.
			 * 
			 * The else branch below handles the opposite scenario where abilities haven't been granted yet.
			 */
			BroadcastAbilityInfo();
		}
		else
		{
			/*
			 * Abilities have NOT been granted yet, so bind our BroadcastAbilityInfo function
			 * to the AbilitiesGivenDelegate, using the `GetFoxASC()` getter function inherited by this class 
			 * (OverlayWidgetController.cpp) to access this delegate. This delegate will be broadcast from 
			 * UFoxAbilitySystemComponent::AddCharacterAbilities() when abilities are granted.
			 * 
			 * AddUObject binds a member function of a UObject-derived class to a delegate.
			 * Parameters: (UObject instance, member function pointer)
			 * 
			 * This handles the timing issue where the widget controller might be initialized BEFORE
			 * abilities are granted (common scenario when UI is created during character initialization).
			 * When abilities are eventually granted, our callback will automatically execute and
			 * broadcast ability information to the UI.
			 */
			GetFoxASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}
		
		/*
		 * Use the getter function that this class inherits to get the FoxAbilitySystemComponent.
		 * Then, bind a lambda callback to the EffectAssetTags delegate, which broadcasts gameplay tag containers
		 * from applied gameplay effects. This delegate is broadcast in UFoxAbilitySystemComponent::ClientEffectApplied
		 * whenever a gameplay effect is applied to this ASC.
		 * 
		 * The lambda captures 'this' to access member variables and functions of UOverlayWidgetController.
		 * The FGameplayTagContainer& AssetTags parameter contains all the asset tags from the applied effect
		 * (retrieved via EffectSpec.GetAllAssetTags() in ClientEffectApplied).
		 * 
		 * This system allows gameplay effects to communicate UI-relevant information to widgets.
		 * For example, when a health potion effect is applied, its asset tags might include "Message.HealthPotion"
		 * which this callback will process to display a corresponding UI message widget.
		 */
		GetFoxASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				/*
				 * Loop through each gameplay tag in the AssetTags container from the applied effect.
				 * const FGameplayTag& makes Tag a const reference, avoiding unnecessary copying of tag data
				 * while preventing modification of the tags.
				 * 
				 * We're searching for tags that represent UI messages (tags with "Message" as parent).
				 */
				for (const FGameplayTag& Tag : AssetTags)
				{
					// For example, say that Tag = Message.HealthPotion
					// "Message.HealthPotion".MatchesTag("Message") will return True, "Message".MatchesTag("Message.HealthPotion") will return False
					/*
					 * Request the "Message" gameplay tag from the project's tag registry.
					 * RequestGameplayTag looks up and returns a tag by name, creating a FGameplayTag handle
					 * that can be used for tag comparisons.
					 * 
					 * FName("Message") converts the string literal "Message" to Unreal's FName type (interned string).
					 * 
					 * This MessageTag acts as a filter - we only want to process tags that are children of "Message"
					 * (e.g., "Message.HealthPotion", "Message.Damage", "Message.LevelUp") because these represent
					 * UI message events that should trigger widget displays.
					 */
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					
					/*
					 * Check if the current Tag from the effect's asset tags is a child of (or matches) the "Message" tag.
					 * 
					 * MatchesTag performs a HIERARCHICAL match - it returns true if Tag is MessageTag OR a child of MessageTag.
					 * For example:
					 *   - FGameplayTag("Message.HealthPotion").MatchesTag(FGameplayTag("Message")) returns TRUE
					 *   - FGameplayTag("Message").MatchesTag(FGameplayTag("Message.HealthPotion")) returns FALSE
					 * 
					 * This directional behavior is intentional - we want to find specific message types (children)
					 * that belong to the Message category (parent), not the other way around.
					 * 
					 * Tags that don't match the Message hierarchy (e.g., "Abilities.Attack", "Status.Stunned") 
					 * are ignored because they don't represent UI message events.
					 */
					if (Tag.MatchesTag(MessageTag))
					{
						/*
						 * Look up the UI widget data associated with this specific message tag from the MessageWidgetDataTable.
						 * 
						 * GetDataTableRowByTag is a templated helper function that searches a data table for a row
						 * where the row's gameplay tag matches the provided Tag parameter.
						 * 
						 * Template parameter: FUIWidgetRow is a struct (defined in blueprint or C++) that contains
						 * UI display information such as:
						 *   - Widget class to spawn
						 *   - Display text/message
						 *   - Icon/image to show
						 *   - Color/duration settings
						 * 
						 * MessageWidgetDataTable is a UDataTable* member variable (likely set in blueprint) that maps
						 * gameplay tags to their corresponding UI widget configurations.
						 * 
						 * For example, the "Message.HealthPotion" tag might map to a row containing:
						 *   - Widget class: WBP_FloatingMessage
						 *   - Text: "+50 Health"
						 *   - Icon: T_HealthPotion_Icon
						 *   - Color: Green
						 * 
						 * Returns a const pointer to the FUIWidgetRow struct, or nullptr if no matching row is found.
						 */
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						
						/*
						 * Broadcast the retrieved UI widget row data to any bound listeners (typically UI widgets).
						 * 
						 * MessageWidgetRowDelegate is a custom multicast delegate (likely declared in the header as
						 * something like: FOnMessageWidgetRowSignature MessageWidgetRowDelegate;) that UI widgets
						 * bind to in order to receive message widget data.
						 * 
						 * The *Row dereferences the pointer to pass the actual FUIWidgetRow struct by value to
						 * all bound delegates. This is safe because we've verified Row is non-null via the data
						 * table lookup (though ideally there should be a null check before dereferencing).
						 * 
						 * When broadcast, any UI widgets listening to this delegate will receive the FUIWidgetRow data
						 * and can create/display the appropriate message widget (e.g., floating damage numbers,
						 * potion pickup notifications, level up messages, etc.).
						 */
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
		}
		);
	}
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	/*
	 * Use the getter function that this class inherits to get the FoxPlayerState.
	 * Then, Retrieve a const pointer to the LevelUpInfo data asset from the FoxPlayerState.
	 * LevelUpInfo is a member variable (set in the FoxPlayerState blueprint) that contains
	 * level progression configuration including XP requirements for each level.
	 */
	const ULevelUpInfo* LevelUpInfo = GetFoxPS()->LevelUpInfo;

	/*
	 * Validate that LevelUpInfo is not null using checkf(), which is a runtime assertion macro.
	 * If LevelUpInfo is null (not set in the blueprint), the program will crash immediately with
	 * the specified error message, helping catch configuration errors during development.
	 */
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out FoxPlayerState Blueprint"));

	/*
	 * Calculate the player's current level based on their XP using FindLevelForXP().
	 * This function searches through the LevelUpInformation array in the data asset to find
	 * which level bracket the NewXP value falls into and returns the corresponding level number.
	 */
	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);

	/*
	 * Get the maximum achievable level by checking the size of the LevelUpInformation array.
	 * Num() returns the number of elements in the array, which corresponds to how many levels
	 * are configured in the LevelUpInfo data asset. We do not use .Num() - 1 since this array has 
	 * an empty placeholder at the 0 index so that level 1 starts at index 1.
	 */
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	
	// Validate that the calculated level is within valid bounds (between 1 and MaxLevel inclusive)
	// to prevent array out-of-bounds access when indexing into LevelUpInformation
	if (Level <= MaxLevel && Level > 0)
	{
		
		/*
		 * Get the CUMULATIVE XP required to reach the NEXT level (Level+1) from level 1.
		 * 
		 * Syntax breakdown of LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement:
		 * - LevelUpInfo: A pointer to the ULevelUpInfo data asset (retrieved from FoxPlayerState)
		 * - ->: Pointer dereference operator to access members of the data asset
		 * - LevelUpInformation: A TArray member variable of the ULevelUpInfo data asset that holds an array of structs
		 * - [Level]: Array index operator accessing the struct element at position 'Level' in the array
		 * - .: Member access operator to access a field within the struct we just retrieved from the array
		 * - LevelUpRequirement: An int32 member variable of the FLevelUpInfo struct representing cumulative XP required
		 * 
		 * The LevelUpInformation array structure:
		 * - Index 0: Empty placeholder (unused, allows level numbers to match array indices)
		 * - Index 1: Data for reaching level 2 (e.g., LevelUpRequirement = 300 total XP needed from level 1)
		 * - Index 2: Data for reaching level 3 (e.g., LevelUpRequirement = 900 total XP needed from level 1)
		 * 
		 * Each LevelUpRequirement value is CUMULATIVE - it represents the TOTAL XP needed from the start
		 * of the game (level 1) to reach that level, NOT the XP needed just within that level bracket.
		 * 
		 * Example: If the player is currently Level 2 (which we calculated from their NewXP):
		 * - LevelUpInformation[2].LevelUpRequirement = 900 (total XP from level 1 to reach level 3)
		 * - This is the XP threshold the player must reach to level up from level 2 to level 3
		 */
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;

		/*
		 * Get the CUMULATIVE XP that was required to reach the player's CURRENT level from level 1.
		 * 
		 * Example: If the player is currently Level 2:
		 * - LevelUpInformation[1].LevelUpRequirement = 300 (total XP from level 1 to reach level 2)
		 * - Any XP beyond this 300 counts as "progress within level 2" toward level 3
		 * 
		 * We access index [Level - 1] because if player is Level 2, we want the XP threshold for reaching Level 2 (not Level 3)
		 * 
		 * Why this matters for the XP bar calculation:
		 * - If player is Level 2 with 500 XP:
		 *   * PreviousLevelUpRequirement = 300 (XP needed to reach level 2)
		 *   * LevelUpRequirement = 900 (XP needed to reach level 3)
		 *   * XP range for level 2 = 900 - 300 = 600 XP
		 *   * Player's progress within level 2 = 500 - 300 = 200 XP
		 *   * XP bar percentage = 200 / 600 = 33.3% progress toward level 3
		 * 
		 * The array's placeholder at index 0 ensures:
		 * - Level 1 players can safely access LevelUpInformation[0] (which returns 0 XP requirement)
		 * - All level numbers directly correspond to their array indices (Level 2 → Index 2, etc.)
		 */
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		// Calculate the XP range for the current level by finding the difference between the XP needed
		// for the next level and the XP needed for the current level. This represents how much XP the
		// player must gain within this level bracket to advance to the next level.
		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		
		
		/*
		 * Calculate how much XP the player has earned WITHIN their current level bracket.
		 * 
		 * NewXP is the player's TOTAL cumulative XP from the start of the game (level 1).
		 * PreviousLevelUpRequirement is the TOTAL cumulative XP that was required to REACH the current level.
		 * 
		 * By subtracting PreviousLevelUpRequirement from NewXP, we get the "excess" XP beyond what was
		 * needed to reach the current level, which represents progress toward the next level.
		 * 
		 * Example: If player is Level 2 with 500 total XP:
		 * - NewXP = 500 (total XP from level 1)
		 * - PreviousLevelUpRequirement = 300 (total XP needed to reach level 2 from level 1)
		 * - XPForThisLevel = 500 - 300 = 200 XP
		 * - This means the player has earned 200 XP "within level 2" toward reaching level 3
		 */
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		/*
		 * Calculate the XP bar fill percentage (0.0 to 1.0) representing the player's progress
		 * through their current level toward the next level.
		 * 
		 * The formula is: (XP earned within this level) / (Total XP range for this level)
		 * 
		 * static_cast<float>() conversions are necessary because:
		 * - Both XPForThisLevel and DeltaLevelRequirement are int32 values
		 * - Integer division in C++ truncates: 200 / 600 = 0 (not 0.333...)
		 * - We need floating-point division to get accurate percentage values
		 * - static_cast<float>() explicitly converts the integers to floats before division
		 * - The result is a float between 0.0 (0% - just reached this level) and 1.0 (100% - about to level up)
		 * 
		 * Example continuing from above (Level 2, 500 total XP):
		 * - XPForThisLevel = 200 XP (progress within level 2)
		 * - DeltaLevelRequirement = 600 XP (total XP range for level 2)
		 * - XPBarPercent = 200.0f / 600.0f = 0.3333f (33.33% progress toward level 3)
		 */
		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		/*
		 * Broadcast the calculated XP bar percentage to all UI widgets bound to this delegate.
		 * 
		 * OnXPPercentChangedDelegate is a multicast delegate (declared in the header) that UI widgets
		 * (such as the XP bar widget) bind to in order to receive XP percentage updates.
		 * 
		 * When broadcast, all bound UI widgets receive the XPBarPercent value (0.0 to 1.0) and can
		 * update their visual display accordingly - typically by filling a progress bar from left to right
		 * where 0.0 = empty bar and 1.0 = full bar.
		 * 
		 * This broadcast happens every time the player gains XP, ensuring the UI stays synchronized
		 * with the player's actual progression. When XPBarPercent reaches 1.0 (100%), the player
		 * levels up (handled elsewhere in the game logic) and the bar resets to show progress
		 * through the new level.
		 */
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
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
}
