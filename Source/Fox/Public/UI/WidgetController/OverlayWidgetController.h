// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/FoxWidgetController.h"
#include "OverlayWidgetController.generated.h"


class UFoxAbilitySystemComponent;
/**
 * @struct FUIWidgetRow
 * @brief Data table row structure that defines configuration data for UI message widgets.
 * 
 * This struct is designed to be used with Unreal Engine's Data Table system (inherits from FTableRowBase).
 * Each row in a Data Table of this type represents a complete configuration for displaying a UI message,
 * including the message content, associated widget class, icon, and a gameplay tag for identification.
 * 
 * Typical workflow:
 * 1. Create a Data Table asset based on this struct in the editor
 * 2. Add rows to the table, each representing a different message type (e.g., "Low Health", "Mana Depleted", "Level Up")
 * 3. Use GetDataTableRowByTag() to retrieve specific rows at runtime based on gameplay events
 * 4. Broadcast the retrieved row via MessageWidgetRowDelegate to create and populate the UI widget
 * 
 * The MessageTag serves as both the unique identifier for the row and the semantic meaning of the message,
 * allowing gameplay code to request specific messages using tags like "Message.StatusEffect.Stunned"
 * without hard-coding row names or indices.
 */
USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * @brief Gameplay tag that uniquely identifies this message and serves as the row lookup key.
	 * 
	 * This tag is used in two ways:
	 * 1. As the row name in the Data Table (the tag's name becomes the row identifier)
	 * 2. As the semantic identifier for the message type in gameplay code
	 * 
	 * For example, a tag "Message.Ability.Cooldown" would identify a cooldown notification message.
	 * When gameplay code needs to display this message, it calls GetDataTableRowByTag() with this tag,
	 * which finds the row whose name matches the tag name.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	/**
	 * @brief The localized text content to display in the message widget.
	 * 
	 * FText supports localization and text formatting, making it suitable for user-facing messages
	 * that may need translation or dynamic content insertion (e.g., "Health: {0}/100").
	 * Content designers populate this in the Data Table editor for each message type.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	/**
	 * @brief The widget class to instantiate and display for this message.
	 * 
	 * TSubclassOf ensures type safety - only classes derived from UFoxUserWidget can be assigned.
	 * Different message types can use different widget blueprints (e.g., simple text popup vs. complex animated banner).
	 * The forward declaration "class UFoxUserWidget" allows this property without including the full header.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UFoxUserWidget> MessageWidget;

	/**
	 * @brief Optional icon/image to display alongside the message.
	 * 
	 * Useful for showing ability icons, status effect symbols, or other visual indicators.
	 * Nullable (initialized to nullptr) because not all messages require an image.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

// Forward declarations
class UFoxUserWidget;
class UAbilityInfo;
struct FOnAttributeChangeData;

/*
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam Macro Explanation:
 * 
 * This Unreal Engine macro creates a dynamic multicast delegate type that can be exposed to Blueprints
 * and allows multiple functions to bind to and receive notifications when the delegate is broadcast.
 * 
 * Syntax Breakdown:
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(DelegateName, ParamType, ParamName)
 * 
 * Parameters:
 * 1. DelegateName: The name of the delegate type being declared (FOnHealthChangedSignature)
 *    - By convention, delegate types start with 'F' and end with 'Signature'
 *    - This becomes a new type that can be used with UPROPERTY declarations
 * 
 * 2. ParamType: The data type of the parameter to pass when broadcasting (float)
 *    - This can be any C++ type (int, float, FString, custom structs, pointers, etc.)
 *    - Multiple parameter variants exist: _OneParam, _TwoParams, _ThreeParams, etc.
 * 
 * 3. ParamName: The name given to the parameter (NewHealth)
 *    - This name appears in Blueprint nodes and helps with code readability
 *    - It's used for documentation and Blueprint UI, not for actual binding
 * 
 * How to Determine the Correct Function Signature for Binding:
 * 
 * To bind a function to this delegate, the function signature must EXACTLY match:
 * 
 * Return Type: void (always for dynamic delegates)
 * Parameters: Must match the delegate's parameter list in order and type
 * 
 * For FOnHealthChangedSignature, the binding function must be:
 * void FunctionName(float NewHealth)
 * 
 * Examples:
 * - void HealthChanged(float NewHealth) const;  // ✓ Correct - matches delegate signature
 * - void UpdateHealthBar(float Value);           // ✓ Correct - parameter name can differ
 * - void OnHealthUpdate(int32 Health);           // ✗ Wrong - parameter type doesn't match (int32 vs float)
 * - float HealthChanged(float NewHealth);        // ✗ Wrong - return type must be void
 * - void HealthChanged();                        // ✗ Wrong - missing required parameter
 * 
 * How to Find the Signature:
 * 1. Look at the macro parameters: OneParam means one parameter
 * 2. The parameter type is the first type argument after the delegate name (float)
 * 3. The function must return void and take exactly one float parameter
 * 4. The parameter name in your function can be anything, but the type must match exactly
 * 
 * Binding Methods:
 * - C++: DelegateName.AddDynamic(Object, &ClassName::FunctionName)
 * - Blueprint: Create custom event with matching signature and bind via node
 * 
 * Broadcasting:
 * - When you call DelegateName.Broadcast(FloatValue), all bound functions receive FloatValue
 */
/*
 * CREATING CUSTOM DELEGATES vs BINDING TO ENGINE DELEGATES
 * 
 * This file demonstrates CREATING custom delegates, which is fundamentally different from
 * binding to existing engine delegates (as seen in FoxEffectActor.h with OnActorBeginOverlap).
 * 
 * Key Differences:
 * 
 * 1. IN THIS FILE (OverlayWidgetController.h) - CREATING CUSTOM DELEGATES:
 *    - We use DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam to CREATE brand new delegate types
 *    - These delegates (FOnHealthChangedSignature, FOnManaChangedSignature, etc.) don't exist
 *      anywhere in the engine until we declare them here
 *    - We define the signature (return type, parameter types) ourselves
 *    - After declaring the type, we instantiate it as a UPROPERTY (OnHealthChanged, OnManaChanged)
 *    - We are the BROADCASTER: we call OnHealthChanged.Broadcast(NewValue) to notify listeners
 *    - Blueprint widgets and other systems can bind TO our custom delegates to receive notifications
 *    - Purpose: Create a custom event system for our specific gameplay needs (health/mana UI updates)
 * 
 * 2. IN FoxEffectActor.h - BINDING TO EXISTING ENGINE DELEGATES:
 *    - The OnActorBeginOverlap delegate already exists as part of AActor class in engine code
 *    - Unreal Engine created this delegate and broadcasts it when overlap events occur
 *    - We DON'T create or declare the delegate type; it's already defined in Actor.h
 *    - We only BIND our callback function to the existing delegate: OnActorBeginOverlap.AddDynamic(...)
 *    - We are the LISTENER: we react when the engine broadcasts the event
 *    - Purpose: Hook into existing engine events (collision, overlap, input, etc.)
 * 
 * Workflow in this file:
 * 1. DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam creates the delegate TYPE (like defining a function pointer signature)
 * 2. UPROPERTY declarations create INSTANCES of those delegate types (OnHealthChanged, OnManaChanged)
 * 3. Blueprint widgets bind to these instances (they become listeners)
 * 4. HealthChanged() callback receives data from AbilitySystemComponent
 * 5. HealthChanged() calls OnHealthChanged.Broadcast(NewValue)
 * 6. All bound Blueprint widgets receive the broadcast and update their UI
 * 
 * Why create custom delegates here?
 * - The Gameplay Ability System has its own delegate system (FOnAttributeChangeData)
 * - Blueprint widgets can't easily bind to GAS delegates (they're not BlueprintAssignable)
 * - We create BlueprintAssignable custom delegates as a "translation layer"
 * - Our controller listens to GAS (via BindCallbacksToDependencies)
 * - Our controller broadcasts to Blueprints (via our custom delegates)
 * - This provides a clean, Blueprint-friendly interface for UI updates
 */
// The above comments need updated. We used to have a delegate for each attribute now we have one delegate for all.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/**
 * Dynamic multicast delegate for broadcasting UI message widget data to listeners.
 * 
 * DELEGATE PURPOSE:
 * This delegate broadcasts complete FUIWidgetRow data structures retrieved from a Data Table,
 * allowing Blueprint UI systems to receive and display message widgets dynamically during gameplay.
 * Unlike the attribute change delegates above (which broadcast simple float values), this delegate
 * broadcasts an entire struct containing all the data needed to create and configure a UI message.
 * 
 * SIGNATURE BREAKDOWN:
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row)
 * - Delegate Type Name: FMessageWidgetRowSignature
 * - Parameter Type: FUIWidgetRow (entire struct passed by value)
 * - Parameter Name: Row (the complete message configuration data)
 * 
 * BINDING FUNCTION SIGNATURE:
 * Any function binding to MessageWidgetRowDelegate must match this signature:
 * void FunctionName(FUIWidgetRow Row)
 * 
 * The function receives the complete row data including MessageTag, Message text, MessageWidget class, and Image.
 * 
 * USAGE PATTERN:
 * 1. Gameplay event occurs (e.g., ability activated, status effect applied, achievement earned)
 * 2. Event handler determines which message to display based on a gameplay tag
 * 3. Handler calls GetDataTableRowByTag() to retrieve the corresponding FUIWidgetRow from MessageWidgetDataTable
 * 4. Handler calls MessageWidgetRowDelegate.Broadcast(RetrievedRow) to notify all listeners
 * 5. Bound Blueprint widgets receive the row data and create/display the specified MessageWidget
 * 6. Blueprint uses Row.Message for text, Row.Image for icon, Row.MessageWidget for widget class, etc.
 * 
 * COMPARISON TO ATTRIBUTE DELEGATES:
 * - Attribute delegates (OnHealthChanged, OnManaChanged): Broadcast simple float values for continuous updates
 * - MessageWidgetRowDelegate: Broadcasts complex structs for discrete, event-driven UI messages
 * - Attribute delegates fire frequently (every gameplay effect that modifies attributes)
 * - MessageWidgetRowDelegate fires on-demand (only when gameplay code explicitly broadcasts a message)
 * 
 * EXAMPLE BROADCAST SCENARIO:
 * Player activates an ability on cooldown:
 * 1. Ability system detects cooldown and retrieves row for tag "Message.Ability.OnCooldown"
 * 2. Broadcasts: MessageWidgetRowDelegate.Broadcast(CooldownRow)
 * 3. UI receives: Row.Message = "Ability on cooldown!", Row.Image = (cooldown icon), Row.MessageWidget = WBP_CooldownPopup
 * 4. UI instantiates WBP_CooldownPopup, sets its text and icon, displays it to player
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row);

/*
 * DELEGATE BINDING FLOW - HOW DATA FLOWS FROM GAS TO BLUEPRINT UI
 * 
 * This controller implements a three-layer delegate system to bridge Gameplay Ability System (GAS)
 * attribute changes to Blueprint UI widgets:
 * 
 * LAYER 1 - GAS ATTRIBUTE CHANGE DELEGATES (Source):
 * - Location: Inside UAbilitySystemComponent (engine code)
 * - Type: FOnGameplayAttributeValueChange (C++ native delegate)
 * - Purpose: The Gameplay Ability System broadcasts attribute changes through these delegates
 * - When fired: Automatically whenever GameplayEffects modify attributes (Health, Mana, etc.)
 * - Broadcast signature: void(const FOnAttributeChangeData& Data)
 * - NOT Blueprint-accessible: These are internal C++ delegates that Blueprints cannot bind to
 * 
 * LAYER 2 - CALLBACK FUNCTIONS (Translation Layer):
 * - Location: Protected section of this class (HealthChanged, ManaChanged, MaxHealthChanged, MaxManaChanged)
 * - Type: Regular C++ member functions
 * - Purpose: Act as intermediaries that receive GAS broadcasts and forward to Blueprint delegates
 * - Binding: Done in BindCallbacksToDependencies() using:
 *   AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::CallbackFunction)
 * - What they do: Extract Data.NewValue from FOnAttributeChangeData and call our custom delegate's Broadcast()
 * - Example: HealthChanged(const FOnAttributeChangeData& Data) receives GAS notification, then calls OnHealthChanged.Broadcast(Data.NewValue)
 * 
 * LAYER 3 - CUSTOM BLUEPRINT DELEGATES (Destination):
 * - Location: Public UPROPERTY section of this class (OnHealthChanged, OnManaChanged, etc.)
 * - Type: Our custom delegate types (FOnHealthChangedSignature, FOnManaChangedSignature, etc.)
 * - Declaration: Created using DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam macros at top of file
 * - Purpose: Provide Blueprint-friendly delegates that UI widgets can bind to
 * - Blueprint-accessible: Marked BlueprintAssignable so Blueprint widgets can bind custom events
 * - Broadcast: Called by callback functions (Layer 2) to notify all listening Blueprint widgets
 * 
 * COMPLETE DATA FLOW EXAMPLE (Health attribute change):
 * 
 * 1. GameplayEffect executes and modifies Health attribute in AttributeSet
 *    ↓
 * 2. AbilitySystemComponent detects change, creates FOnAttributeChangeData struct with OldValue/NewValue
 *    ↓
 * 3. ASC broadcasts to its internal FOnGameplayAttributeValueChange delegate for Health attribute
 *    ↓
 * 4. Our HealthChanged() callback function receives the broadcast (bound in BindCallbacksToDependencies)
 *    ↓
 * 5. HealthChanged() extracts Data.NewValue and calls OnHealthChanged.Broadcast(Data.NewValue)
 *    ↓
 * 6. All Blueprint widgets bound to OnHealthChanged receive the new value and update their UI
 * 
 * WHY THREE LAYERS?
 * - GAS uses C++ delegates (Layer 1) that Blueprints cannot directly access
 * - Callback functions (Layer 2) bridge the gap between C++ and Blueprint systems
 * - Custom delegates (Layer 3) provide a clean, Blueprint-friendly API for UI designers
 * - This pattern separates engine internals (GAS) from game-specific UI logic (Blueprints)
 * - Allows UI designers to work in Blueprints without understanding GAS implementation details
 * 
 * BINDING SETUP:
 * - BindCallbacksToDependencies(): Connects Layer 1 (GAS) to Layer 2 (callbacks)
 * - Blueprint Event Graph: Connects Layer 3 (custom delegates) to UI update logic
 * - Both bindings happen once during initialization and remain active throughout gameplay
 */
/**
 * @class UOverlayWidgetController
 * @brief A controller class for managing overlay widgets in the FOX game framework.
 *
 * This class extends the functionality of the UFoxWidgetController and adds
 * methods and properties specific to handling attributes such as health and mana
 * for UI overlay widgets.
 */
UCLASS(BlueprintType, Blueprintable)
class FOX_API UOverlayWidgetController : public UFoxWidgetController
{
	GENERATED_BODY()
	
public:
	
	/**
	 * @brief Broadcasts the initial values of all attributes to update the UI immediately upon widget creation.
	 * 
	 * This function is called after the widget controller has been initialized and connected to the widget.
	 * It retrieves current attribute values (Health, MaxHealth, Mana, MaxMana) from the AttributeSet
	 * and broadcasts them through their respective delegates, ensuring the UI displays accurate
	 * data from the first frame rather than showing default/uninitialized values.
	 */
	virtual void BroadcastInitialValues() override;
	
	/**
	 * @brief Registers callback functions to listen for attribute changes from the Ability System Component.
	 * 
	 * This function binds the controller's internal callback methods (HealthChanged, MaxHealthChanged,
	 * ManaChanged, MaxManaChanged) to the corresponding attribute change delegates in the
	 * AbilitySystemComponent. Once bound, these callbacks will automatically fire whenever
	 * attribute values change during gameplay, allowing the controller to broadcast updates to the UI.
	 */
	virtual void BindCallbacksToDependencies() override;
	
	/**
	 * @brief Delegate that broadcasts health value changes to Blueprint-based UI widgets.
	 * 
	 * This BlueprintAssignable delegate allows Blueprint widgets to bind custom events that respond
	 * to health changes in real-time. When the HealthChanged() callback is triggered by the
	 * AbilitySystemComponent, this delegate fires with the new health value, enabling UI elements
	 * (such as health bars or text displays) to update dynamically during gameplay.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;
	
	/**
	 * @brief Delegate that broadcasts maximum health value changes to Blueprint-based UI widgets.
	 * 
	 * See OnHealthChanged property documentation above for detailed explanation of attribute change delegates.
	 * This delegate functions identically but broadcasts MaxHealth attribute changes instead of current Health.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	/**
	 * @brief Delegate that broadcasts mana value changes to Blueprint-based UI widgets.
	 * 
	 * See OnHealthChanged property documentation above for detailed explanation of attribute change delegates.
	 * This delegate functions identically but broadcasts Mana attribute changes instead of Health.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	/**
	 * @brief Delegate that broadcasts maximum mana value changes to Blueprint-based UI widgets.
	 * 
	 * See OnHealthChanged property documentation above for detailed explanation of attribute change delegates.
	 * This delegate functions identically but broadcasts MaxMana attribute changes instead of current Health.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	/**
	 * @brief Delegate that broadcasts complete UI message widget data to Blueprint listeners.
	 * 
	 * See FMessageWidgetRowSignature delegate declaration comments above for detailed explanation of
	 * this delegate's purpose, signature, usage pattern, and how it differs from attribute change delegates.
	 * This property is the actual delegate instance that gameplay code broadcasts to, while FMessageWidgetRowSignature
	 * defines the delegate type itself.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate;
	
	/**
	 * Delegate that broadcasts experience point (XP) percentage changes to Blueprint UI systems.
	 * Uses FOnAttributeChangedSignature (same type as health/mana delegates) which broadcasts a single float value.
	 * Any function binding to this delegate must match: void FunctionName(float NewValue)
	 * The NewValue parameter represents XP percentage: 0.0 = no progress, 0.5 = halfway to next level, 1.0 = ready to level up.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|XP")
	FOnAttributeChangedSignature OnXPPercentChangedDelegate;
	
	/**
	 * Delegate that broadcasts player level changes to Blueprint UI systems.
	 * Uses FOnPlayerStatChangedSignature which broadcasts a single int32 value.
	 * Any function binding to this delegate must match: void FunctionName(int32 NewValue)
	 * The NewValue parameter represents the new player level (e.g., 1, 2, 3, etc.).
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS|Level")
	FOnPlayerStatChangedSignature OnPlayerLevelChangedDelegate;
	
protected:
	
	/**
	 * @brief Data Table asset containing all configured UI message widget definitions.
	 * 
	 * This Data Table uses FUIWidgetRow as its row structure, with each row representing a different
	 * message type that can be displayed to the player. The table is configured in the editor and
	 * serves as a centralized repository for all UI message configurations, allowing designers to
	 * add, modify, or remove messages without changing code.
	 * 
	 * CONFIGURATION:
	 * - EditDefaultsOnly: Can only be set on the class default object (CDO), not on individual instances
	 * - BlueprintReadOnly: Blueprint subclasses can read but not modify this property
	 * - Category="Widget Data": Organizes this property in the editor details panel
	 * 
	 * DATA TABLE STRUCTURE:
	 * Each row in the table has:
	 * - Row Name: Must match the MessageTag's tag name (e.g., "Message.Ability.OnCooldown")
	 * - MessageTag: The gameplay tag identifying this message type
	 * - Message: The localized text to display
	 * - MessageWidget: The widget blueprint class to instantiate
	 * - Image: Optional icon/image to show with the message
	 * 
	 * LOOKUP MECHANISM:
	 * The GetDataTableRowByTag() method searches this table using a gameplay tag's name as the row identifier.
	 * This design requires that:
	 * 1. Each row's name in the Data Table matches its MessageTag's tag name exactly
	 * 2. Tags are unique within the table (no duplicate MessageTag values)
	 * 
	 * TYPICAL SETUP:
	 * 1. Create a Data Table asset in the editor with FUIWidgetRow as the row structure
	 * 2. Add rows for each message type your game needs:
	 *    - Row: "Message.Ability.OnCooldown" → MessageTag: Message.Ability.OnCooldown, Message: "On Cooldown!", etc.
	 *    - Row: "Message.Health.Critical" → MessageTag: Message.Health.Critical, Message: "Low Health!", etc.
	 *    - Row: "Message.Achievement.Unlocked" → MessageTag: Message.Achievement.Unlocked, Message: "Achievement!", etc.
	 * 3. Assign this Data Table asset to this property in the OverlayWidgetController's class defaults
	 * 4. At runtime, call GetDataTableRowByTag() with any of these tags to retrieve the corresponding row
	 * 
	 * USAGE EXAMPLE:
	 * if (FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, CooldownTag))
	 * {
	 *     MessageWidgetRowDelegate.Broadcast(*Row); // Notify UI to display the cooldown message
	 * }
	 * 
	 * ADVANTAGES OF DATA TABLE APPROACH:
	 * - Centralizes all message configurations in one asset
	 * - Allows non-programmers to add/edit messages without code changes
	 * - Supports localization through FText properties
	 * - Enables different widget classes for different message types
	 * - Messages can be easily reused across different gameplay systems
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;
	
	/**
	 * @brief Template method to retrieve a Data Table row using a gameplay tag as the lookup key.
	 * 
	 * This generic utility method searches a Data Table for a row whose name matches the provided
	 * gameplay tag's name, returning a pointer to the row data cast to the specified type.
	 * 
	 * TEMPLATE PARAMETER:
	 * @tparam T The struct type that defines the row structure (must inherit from FTableRowBase)
	 *           Common usage: GetDataTableRowByTag<FUIWidgetRow>(DataTable, Tag)
	 *           The template allows this method to work with any Data Table structure, not just FUIWidgetRow.
	 * 
	 * PARAMETERS:
	 * @param DataTable Pointer to the Data Table asset to search. Should not be null.
	 * @param Tag The gameplay tag whose name will be used as the row lookup key.
	 *            The tag's name (from Tag.GetTagName()) must exactly match a row name in the table.
	 * 
	 * RETURN VALUE:
	 * @return T* Pointer to the found row data cast to type T, or nullptr if:
	 *         - DataTable is null
	 *         - No row exists with a name matching Tag.GetTagName()
	 *         - The row exists but type T doesn't match the table's row structure
	 * 
	 * KEY DESIGN DECISION:
	 * This method assumes that Data Table row names match their corresponding gameplay tag names.
	 * For example:
	 * - Gameplay Tag: "Message.Ability.OnCooldown"
	 * - Row Name: "Message.Ability.OnCooldown" (must match exactly)
	 * - Row's MessageTag property: Message.Ability.OnCooldown (same tag)
	 * 
	 * This convention creates a direct mapping between tags and rows, enabling efficient lookups
	 * without iterating through all rows to compare MessageTag properties. The tag itself serves
	 * as both the semantic identifier and the physical row key.
	 * 
	 * IMPLEMENTATION NOTES:
	 * - Uses UDataTable::FindRow() internally, which performs a fast FName-based hash lookup
	 * - Second parameter to FindRow() is an optional context string for error messages (empty here)
	 * - FindRow() returns nullptr if the row doesn't exist, which this method passes through to caller
	 * - Caller is responsible for null-checking the return value before dereferencing
	 * 
	 * USAGE EXAMPLE:
	 * FGameplayTag CooldownMessageTag = FGameplayTag::RequestGameplayTag("Message.Ability.OnCooldown");
	 * if (FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, CooldownMessageTag))
	 * {
	 *     // Row found - Row->Message contains "Ability on Cooldown!", Row->Image contains icon, etc.
	 *     MessageWidgetRowDelegate.Broadcast(*Row);
	 * }
	 * else
	 * {
	 *     // Row not found - either DataTable is null, or no row named "Message.Ability.OnCooldown" exists
	 *     UE_LOG(LogFox, Warning, TEXT("Failed to find message row for tag: %s"), *CooldownMessageTag.ToString());
	 * }
	 * 
	 * WHY TEMPLATE METHOD:
	 * - Provides type safety through compile-time checking
	 * - Reusable with any Data Table structure, not limited to FUIWidgetRow
	 * - Could be used for other systems (ability data, item data, enemy data, etc.)
	 * - Avoids casting boilerplate in calling code
	 * 
	 * COMPARISON TO ABILITY SYSTEM APPROACH:
	 * AbilityInfo.cpp uses a different pattern - iterating through an array and comparing tags in a loop.
	 * This method leverages Unreal's Data Table system, which uses hashed row names for O(1) lookup.
	 * Trade-off: Requires row names match tag names, but provides significantly faster lookups for large datasets.
	 */
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
	
	/**
	 * @brief Callback function bound to AFoxPlayerState::OnXPChangedDelegate that converts raw XP values into percentage progress.
	 * 
	 * This function is bound to AFoxPlayerState::OnXPChangedDelegate, which is a
	 * multicast delegate that broadcasts whenever the player's XP changes. When AFoxPlayerState modifies the XP value
	 * (via AddToXP or SetXP methods), the delegate broadcasts and calls this function, passing the new XP value as a parameter.
	 * The function receives the raw XP value, converts it into a percentage representing progress toward the next level
	 * (using data from the LevelUpInfo Data Asset), and broadcasts the result to Blueprint UI widgets via 
	 * OnXPPercentChangedDelegate, enabling XP bars to display visual progress (0.0 = no progress, 1.0 = ready to level up).
	 */
	void OnXPChanged(int32 NewXP);
	
	/**
	 * @brief Callback function bound to UFoxAbilitySystemComponent's ability equipped delegate that notifies UI of ability slot assignments.
	 * 
	 * This function is bound to the Ability System Component's OnAbilityEquipped delegate
	 * and is invoked whenever an ability is equipped to an input slot or moved between slots. It receives comprehensive information
	 * about the equipment change including which ability was equipped, its current status (locked/unlocked/eligible), the new slot
	 * assignment, and the previous slot (if the ability was moved rather than newly equipped). This callback serves as a bridge
	 * between the gameplay ability system and Blueprint UI widgets, forwarding equipment events so that ability bar UI can update
	 * to reflect current slot assignments, ability icons, cooldowns, and input bindings.
	 * 
	 * PARAMETERS:
	 * @param AbilityTag Gameplay tag identifying which ability was equipped (e.g., "Abilities.Fire.Fireball", "Abilities.Lightning.Electrocute").
	 *                   This tag uniquely identifies the ability within the game's ability system and is used by UI to retrieve
	 *                   ability information (icon, name, description) from ability data assets for display purposes.
	 * 
	 * @param Status Gameplay tag indicating the ability's current status in relation to the player (e.g., "Abilities.Status.Equipped",
	 *               "Abilities.Status.Locked", "Abilities.Status.Unlocked", "Abilities.Status.Eligible"). UI uses this to determine
	 *               visual presentation - equipped abilities show normally, locked abilities appear grayed out/disabled, unlocked
	 *               abilities might show "available to equip" indicators, and eligible abilities could display level requirements.
	 * 
	 * @param Slot Gameplay tag representing the input slot where the ability is now equipped (e.g., "InputTag.LMB", "InputTag.RMB",
	 *             "InputTag.1", "InputTag.2", "InputTag.Q"). This maps directly to player input bindings, allowing UI to display
	 *             which key/button triggers which ability. The tag hierarchy typically matches input action names defined in
	 *             the project's input mapping context, creating a direct correspondence between UI display and gameplay input.
	 * 
	 * @param PreviousSlot Gameplay tag indicating the slot where the ability was previously equipped, or an empty/invalid tag if
	 *                     the ability is being equipped for the first time (not moved from another slot). This parameter enables
	 *                     UI to handle ability swaps and moves - if PreviousSlot is valid, the UI can clear the old slot's display
	 *                     before populating the new slot, ensuring no duplicate ability icons appear. For initial equipment,
	 *                     PreviousSlot.IsValid() returns false, signaling this is a new assignment rather than a move operation.
	 */
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const;
};

/**
 * @brief Template method to retrieve a Data Table row using a gameplay tag as the lookup key.
 * 
 * This generic utility method searches a Data Table for a row whose name matches the provided
 * gameplay tag's name, returning a pointer to the row data cast to the specified type.
 * 
 * TEMPLATE PARAMETER:
 * @tparam T The struct type that defines the row structure (must inherit from FTableRowBase)
 *		   Common usage: GetDataTableRowByTag<FUIWidgetRow>(DataTable, Tag)
 *		   The template allows this method to work with any Data Table structure, not just FUIWidgetRow.
 * 
 * PARAMETERS:
 * @param DataTable Pointer to the Data Table asset to search. Should not be null.
 * @param Tag The gameplay tag whose name will be used as the row lookup key.
 *			The tag's name (from Tag.GetTagName()) must exactly match a row name in the table.
 * 
 * RETURN VALUE:
 * @return T* Pointer to the found row data cast to type T, or nullptr if:
 *		 - DataTable is null
 *		 - No row exists with a name matching Tag.GetTagName()
 *		 - The row exists but type T doesn't match the table's row structure
 * 
 * KEY DESIGN DECISION:
 * This method assumes that Data Table row names match their corresponding gameplay tag names.
 * For example:
 * - Gameplay Tag: "Message.Ability.OnCooldown"
 * - Row Name: "Message.Ability.OnCooldown" (must match exactly)
 * - Row's MessageTag property: Message.Ability.OnCooldown (same tag)
 * 
 * This convention creates a direct mapping between tags and rows, enabling efficient lookups
 * without iterating through all rows to compare MessageTag properties. The tag itself serves
 * as both the semantic identifier and the physical row key.
 * 
 * IMPLEMENTATION NOTES:
 * - Uses UDataTable::FindRow() internally, which performs a fast FName-based hash lookup (O(1) complexity)
 * - Second parameter to FindRow() is an optional context string for error messages (empty here)
 * - FindRow() returns nullptr if the row doesn't exist, which this method passes through to caller
 * - Caller is responsible for null-checking the return value before dereferencing
 * 
 * USAGE EXAMPLE:
 * @code
 * FGameplayTag CooldownMessageTag = FGameplayTag::RequestGameplayTag("Message.Ability.OnCooldown");
 * if (FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, CooldownMessageTag))
 * {
 *	 // Row found - Row->Message contains "Ability on Cooldown!", Row->Image contains icon, etc.
 *	 MessageWidgetRowDelegate.Broadcast(*Row);
 * }
 * else
 * {
 *	 // Row not found - either DataTable is null, or no row named "Message.Ability.OnCooldown" exists
 *	 UE_LOG(LogFox, Warning, TEXT("Failed to find message row for tag: %s"), *CooldownMessageTag.ToString());
 * }
 * @endcode
 * 
 * WHY TEMPLATE METHOD:
 * - Provides type safety through compile-time checking
 * - Reusable with any Data Table structure, not limited to FUIWidgetRow
 * - Could be used for other systems (ability data, item data, enemy data, etc.)
 * - Avoids casting boilerplate in calling code
 * 
 * COMPARISON TO AbilityInfo APPROACH:
 * AbilityInfo.cpp's FindAbilityInfoForTag() iterates through a TArray comparing tags in a loop (O(n) complexity).
 * This method leverages UDataTable::FindRow() which uses hashed row names for O(1) lookup performance.
 * Trade-off: Requires row names match tag names, but provides significantly faster lookups for large datasets,
 * making it ideal for frequently-accessed data like UI messages that may be triggered multiple times per frame.
 */
template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	// Call UDataTable::FindRow<T>() to search the Data Table for a row with a name matching the gameplay tag's name.
	//
	// PARAMETERS:
	// 1. Tag.GetTagName() - Returns the tag's FName (e.g., "Message.Ability.OnCooldown") used as the row lookup key.
	//    This method relies on the convention that Data Table row names match their corresponding tag names exactly.
	//    For example, if Tag contains "Message.Health.Critical", FindRow searches for a row named "Message.Health.Critical".
	//
	// 2. TEXT("") - The context string parameter for error/warning messages (passed as empty string here).
	//    When FindRow fails to locate a row, Unreal can log a warning that includes this context string to help debugging.
	//    We pass an empty string because the header's detailed documentation already explains the lookup mechanism,
	//    and callers are expected to perform their own null-checking and error handling (see usage examples in header).
	//    Providing context here would generate redundant log spam for expected "not found" cases during gameplay.
	//
	// RETURN VALUE:
	// Returns T* (pointer to row data cast to template type T) if a matching row exists, or nullptr if:
	// - DataTable is null
	// - No row exists with name matching Tag.GetTagName()
	// - Row exists but type T doesn't match the table's row structure
	//
	// COMPARISON TO AbilityInfo APPROACH:
	// AbilityInfo.cpp uses FindAbilityInfoForTag() which iterates through a TArray comparing tags in a loop (O(n) complexity).
	// This method uses FindRow() which performs a hashed FName lookup (O(1) complexity) for significantly better performance.
	// Trade-off: Requires strict row naming convention (row name must match tag name), but provides instant lookups even
	// with thousands of rows, making it ideal for frequently-accessed data like UI messages that trigger every frame.
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}
