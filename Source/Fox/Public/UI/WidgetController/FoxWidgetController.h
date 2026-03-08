// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FoxWidgetController.generated.h"

class UAbilityInfo;
class UFoxAttributeSet;
class UFoxAbilitySystemComponent;
class AFoxPlayerState;
class AFoxPlayerController;
// Creates a Blueprint bindable delegate type named FOnPlayerStatChangedSignature that broadcasts an int32 value (NewValue)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);

/**
 * This delegate broadcasts FFoxAbilityInfo structs containing complete ability configuration data
 * (tags, icons, materials) to Blueprint UI systems, enabling dynamic population of ability bars,
 * tooltips, and other ability-related UI widgets during gameplay initialization and when abilities are granted.
 * 
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FFoxAbilityInfo&, Info)
 * - Delegate Type Name: FAbilityInfoSignature
 * - Parameter Type: const FFoxAbilityInfo& (struct passed by const reference for efficiency)
 * - Parameter Name: Info (the complete ability configuration data including AbilityTag, InputTag, Icon, and BackgroundMaterial)
 *
 * Any function binding to AbilityInfoDelegate must match this signature:
 * void FunctionName(const FFoxAbilityInfo& Info)
 * 
 * When this delegate broadcasts, the function that binds to it receives the complete ability info including AbilityTag,
 * InputTag, Icon, and BackgroundMaterial.
 * 
 * USAGE PATTERN:
 * 1. Ability system initialization completes (OnInitializeStartupAbilities callback fires)
 * 2. This Widget Controller iterates through granted abilities in the FoxAbilitySystemComponent
 * 3. For each ability, controller retrieves FFoxAbilityInfo from AbilityInfo Data Asset using FindAbilityInfoForTag()
 * 4. Controller calls AbilityInfoDelegate.Broadcast(RetrievedInfo) to notify all listeners
 * 5. Bound Blueprint widgets receive the info and populate ability bars/slots with icons, backgrounds, and input bindings
 * 6. Blueprint uses Info.Icon for ability icon, Info.BackgroundMaterial for button material, Info.InputTag for hotkey display
 * 
 * EXAMPLE BROADCAST SCENARIO:
 * Player character initialization grants startup abilities:
 * 1. FoxAbilitySystemComponent finishes granting abilities and broadcasts AbilitiesGivenDelegate
 * 2. OnInitializeStartupAbilities callback receives the ASC and iterates through GetActivatableAbilities()
 * 3. For each ability spec, retrieves: Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag)
 * 4. Broadcasts: AbilityInfoDelegate.Broadcast(Info)
 * 5. UI receives: Info.AbilityTag = "Abilities.Fire.Fireball", Info.InputTag = "InputTag.1", Info.Icon = (fireball texture)
 * 6. UI creates ability button in slot 1, sets icon texture, sets background material, binds to hotkey 1
 * 
 * This delegate uses const reference (const FFoxAbilityInfo&)
 * for performance optimization. FFoxAbilityInfo contains UObject pointers (Icon, BackgroundMaterial) that are cheap to copy,
 * but using const reference avoids unnecessary struct copying and follows C++ best practices for passing non-trivial types.
 * The const qualifier ensures receivers cannot modify the source data, maintaining data integrity.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FFoxAbilityInfo&, Info);

class UAttributeSet;
class UAbilitySystemComponent;

/*
 * FWidgetControllerParams - A parameter container struct for initializing Widget Controllers
 * 
 * WHY THIS STRUCT EXISTS:
 * This struct serves as a "parameter bundle" to pass multiple related objects to a Widget Controller in a single, organized package.
 * Instead of passing 4 separate parameters (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet) to initialization functions,
 * we bundle them into one struct. This makes the code cleaner, more maintainable, and easier to extend if we need to add more parameters later.
 * It follows the "Parameter Object" design pattern, which reduces function signature complexity and makes the API easier to use.
 * 
 * WHY TWO CONSTRUCTORS:
 * 1. Default Constructor: FWidgetControllerParams() {}
 *    - This is an empty constructor that creates a struct with all members set to their default values (nullptr in this case)
 *    - Useful when you want to create the struct first and set the values later, or when Unreal's reflection system needs to instantiate it
 *    - Required by Unreal Engine for USTRUCT types that need to be created dynamically or used in Blueprints
 * 
 * 2. Parameterized Constructor: FWidgetControllerParams(APlayerController* PC, APlayerState* PS, ...)
 *    - This constructor allows you to create and initialize the struct in one line with all values provided
 *    - Convenient when you have all the required objects ready and want to pass them immediately
 *    - Example usage: FWidgetControllerParams Params(MyPC, MyPS, MyASC, MyAS);
 * 
 * THE COLON-SEPARATED LIST (Member Initialization List):
 * The syntax after the constructor's closing parenthesis and before the opening brace is called a "member initialization list"
 * Example: : PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS)
 * 
 * - This is C++ syntax for initializing member variables when the object is constructed
 * - It's more efficient than assigning values inside the constructor body because it initializes members directly
 *   rather than first default-constructing them and then assigning new values
 * - The format is: : MemberName(ConstructorParameter), AnotherMember(AnotherParameter)
 * - In this case, it assigns the constructor parameters (PC, PS, ASC, AS) to the corresponding struct members
 * - This is considered best practice in C++ for constructor initialization
 * 
 * USTRUCT(BlueprintType):
 * - USTRUCT() is an Unreal Engine macro that marks this as a reflected struct (visible to Unreal's property system)
 * - BlueprintType means this struct can be used as a variable type in Blueprint scripts
 * - Without BlueprintType, you couldn't create Blueprint variables of this struct type or pass it between Blueprint nodes
 * - This allows designers and Blueprint programmers to work with this data structure visually in the editor
 * - The struct members marked with UPROPERTY(BlueprintReadWrite) can be accessed and modified in Blueprints
 * 
 * USAGE EXAMPLE:
 * // Create with all parameters at once:
 * FWidgetControllerParams Params(PC, PS, ASC, AS);
 * WidgetController->SetWidgetControllerParams(Params);
 * 
 * // Or create empty and set later:
 * FWidgetControllerParams Params;
 * Params.PlayerController = PC;
 * Params.PlayerState = PS;
 * // ... etc
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	
	FWidgetControllerParams() {}
	
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS) 
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/*
 * UFoxWidgetController - Base Widget Controller Class
 * 
 * WHAT THIS CLASS IS FOR:
 * This class serves as the Controller in an MVC (Model-View-Controller) architecture for UI widgets.
 * It acts as an intermediary layer between gameplay systems (Model) and UI widgets (View), managing
 * data flow and translating gameplay events into UI-friendly updates.
 * 
 * The widget controller's primary responsibilities are:
 * 1. Hold references to core gameplay systems (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet)
 * 2. Listen for changes in gameplay data (like attribute value changes from the Gameplay Ability System)
 * 3. Broadcast UI-friendly events when those changes occur, which widgets can listen to
 * 4. Provide initial values to widgets when they're first displayed
 * 5. Keep UI logic separate from gameplay logic, making both more maintainable and testable
 * 
 * WHY USE A WIDGET CONTROLLER:
 * Without this controller, widgets would need to directly interact with gameplay systems like AbilitySystemComponent
 * and AttributeSet. This creates tight coupling and makes it hard to reuse widgets with different gameplay systems.
 * 
 * The controller pattern provides a clean separation: gameplay systems don't know about UI,
 * and UI widgets don't know about the complexity of gameplay systems.
 * 
 * WHAT IS UObject:
 * UObject is the base class for all objects in Unreal Engine that need to be managed by Unreal's
 * object system and garbage collector. It's the foundation of Unreal's reflection system.
 * 
 * Key features UObject provides:
 * 1. AUTOMATIC MEMORY MANAGEMENT (Garbage Collection):
 *    - Unreal tracks all UObject references and automatically deletes objects when no longer referenced
 *    - You never call delete on UObjects; the engine handles cleanup for you
 *    - This prevents memory leaks and dangling pointers
 * 
 * 2. REFLECTION SYSTEM:
 *    - UObject enables the UPROPERTY, UFUNCTION, and UCLASS macros to work
 *    - Allows C++ classes and their members to be visible to Blueprints
 *    - Enables serialization (saving/loading), replication (networking), and editor integration
 * 
 * 3. BLUEPRINT INTEGRATION:
 *    - Classes derived from UObject can be extended in Blueprints
 *    - Properties marked with UPROPERTY can be accessed in Blueprint scripts
 *    - Functions marked with UFUNCTION can be called from Blueprints
 * 
 * 4. NETWORK REPLICATION:
 *    - UObject provides the foundation for replicating data across networked games
 *    - Properties can be marked for automatic synchronization between server and clients
 * 
 * 5. SERIALIZATION:
 *    - UObjects can be automatically saved to disk and loaded back
 *    - Essential for save games, asset management, and editor functionality
 * 
 * 6. EDITOR INTEGRATION:
 *    - Properties marked with EditAnywhere or EditDefaultsOnly appear in the Unreal Editor
 *    - Allows designers to configure objects visually without touching code
 * 
 * WHY UFoxWidgetController INHERITS FROM UObject:
 * Widget controllers don't need to be Actors (which exist in the world) or ActorComponents (which attach to Actors).
 * They're pure data management objects that need:
 * - Garbage collection (so we don't have to manually delete them)
 * - Blueprint support (so designers can create Blueprint subclasses with custom behavior)
 * - The ability to hold UPROPERTY references to other UObjects
 * - Access to Unreal's object creation system (NewObject<>)
 * 
 * UObject is the lightest-weight Unreal class that provides these features, making it perfect for
 * non-spatial game objects like controllers, subsystems, and data managers.
 * 
 * USAGE PATTERN:
 * 1. Create the controller: NewObject<UFoxWidgetController>()
 * 2. Initialize it: SetWidgetControllerParams() with gameplay system references
 * 3. Bind to data sources: BindCallbacksToDependencies() to listen for attribute changes
 * 4. Broadcast initial state: BroadcastInitialValues() to populate UI with current data
 * 5. Widget listens: UI widgets bind to the controller's broadcast delegates and update automatically
 * 
 * This class is meant to be subclassed (like UOverlayWidgetController) to provide specific
 * functionality for different UI elements (overlay HUD, inventory, character sheet, etc.).
 */
UCLASS()
class FOX_API UFoxWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	
	/*
	 * SetWidgetControllerParams - Initialize the Widget Controller with Core Gameplay System References
	 * 
	 * PURPOSE:
	 * This function stores references to the four essential gameplay systems (PlayerController, PlayerState,
	 * AbilitySystemComponent, AttributeSet) that the widget controller needs to function. It's the first step
	 * in the initialization sequence and must be called before BindCallbacksToDependencies() or BroadcastInitialValues().
	 * 
	 * The function takes a FWidgetControllerParams struct containing all necessary references and unpacks them
	 * into the controller's member variables, making them accessible throughout the controller's lifetime.
	 * 
	 * PARAMETERS:
	 * @param WCParams - A const reference to FWidgetControllerParams struct containing:
	 *   - PlayerController: Used for player input handling and viewport management
	 *   - PlayerState: Persistent player-specific data and typically owns the AbilitySystemComponent
	 *   - AbilitySystemComponent: Manages gameplay abilities, effects, and attribute changes
	 *   - AttributeSet: Contains the actual attribute values (Health, Mana, etc.) that the UI displays
	 * 
	 * USAGE:
	 * Called by AFoxHUD::GetOverlayWidgetController() immediately after creating a new widget controller instance
	 * to provide it with access to gameplay systems before binding callbacks or broadcasting values.
	 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);
	
	/*
	 * BroadcastInitialValues - Send Current Attribute Values to UI Widgets on Initialization
	 * 
	 * PURPOSE:
	 * This virtual function is responsible for broadcasting the current state of all relevant gameplay attributes
	 * to UI widgets when they are first displayed. It ensures that widgets show accurate, up-to-date information
	 * immediately upon appearing, rather than displaying default/zero values until the first attribute change occurs.
	 * 
	 * WHY THIS IS NEEDED:
	 * Widget controllers set up callbacks that fire when attributes CHANGE, but these callbacks won't trigger
	 * for the initial values that already exist. Without this function, a health bar would show 0/0 until
	 * the player takes damage or heals, creating a poor user experience and confusing initial state.
	 * 
	 * IMPLEMENTATION:
	 * This base class version is empty and meant to be overridden by derived classes (like UOverlayWidgetController).
	 * Each subclass implements its own version to broadcast the specific attributes it manages by directly
	 * reading current values from the AttributeSet and firing the appropriate broadcast delegates.
	 * 
	 * CALL ORDER:
	 * Must be called AFTER:
	 * 1. SetWidgetControllerParams() - so the controller has references to gameplay systems
	 * 2. BindCallbacksToDependencies() - so widgets have subscribed to the broadcast delegates
	 * 3. Widget->SetWidgetController() - so the widget can receive the broadcasts
	 * 
	 * Called from AFoxHUD::InitOverlay() as the final initialization step before adding the widget to viewport.
	 */
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	
	/*
	 * BindCallbacksToDependencies - Register Callbacks to Listen for Attribute Changes
	 * 
	 * PURPOSE:
	 * This virtual function sets up callback listeners that monitor the AbilitySystemComponent for attribute changes
	 * (Health, MaxHealth, Mana, MaxMana, etc.) and automatically trigger UI updates when those changes occur.
	 * It establishes the real-time data binding between gameplay systems and UI widgets.
	 * 
	 * HOW IT WORKS:
	 * The function uses the Gameplay Ability System's delegate system to register callbacks on specific attribute
	 * change events. When an attribute value changes (due to damage, healing, buffs, etc.), the AbilitySystemComponent
	 * fires the corresponding delegate, which triggers the callback function in the widget controller. The controller
	 * then processes the new value and broadcasts it to listening UI widgets via its own delegates.
	 * 
	 * IMPLEMENTATION:
	 * This base class version is empty and meant to be overridden by derived classes (like UOverlayWidgetController).
	 * Each subclass implements its own version to bind callbacks for the specific attributes it manages, using
	 * functions like AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
	 * to obtain the appropriate delegates to bind to.
	 * 
	 * CALL ORDER:
	 * Must be called AFTER:
	 * - SetWidgetControllerParams() - so the controller has valid references to AbilitySystemComponent and AttributeSet
	 * Must be called BEFORE:
	 * - BroadcastInitialValues() - so the bindings are ready to handle initial value broadcasts
	 * 
	 * Called from AFoxHUD::GetOverlayWidgetController() immediately after SetWidgetControllerParams() during
	 * the widget controller's lazy initialization sequence.
	 */
	virtual void BindCallbacksToDependencies();
	
	/**
	 * @brief Instance of FAbilityInfoSignature delegate that broadcasts ability information to Blueprint UI systems.
	 * 
	 * See FAbilityInfoSignature delegate type declaration comments above for detailed explanation of this delegate's
	 * purpose, signature, usage pattern, and data flow. This property is the actual delegate instance that
	 * OnInitializeStartupAbilities() broadcasts to when startup abilities are granted, allowing Blueprint widgets
	 * to bind custom events that receive FFoxAbilityInfo structs and populate ability bars accordingly.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;
	
	/* 
	 * Loops through all abilities granted to the player's AbilitySystemComponent, retrieves their
	 * display information (icons, input bindings, etc.) from the AbilityInfo data asset, and broadcasts
	 * each ability's info via AbilityInfoDelegate so UI widgets can populate ability bars and tooltips.
	 * Typically called during UI initialization or when abilities are dynamically granted/changed.
	*/
	void BroadcastAbilityInfo();
	
protected:
	
	/**
	 * @brief Data Asset containing all configured ability information for UI display purposes.
	 * 
	 * This Data Asset stores an array of FFoxAbilityInfo structs, with each entry representing
	 * a different ability that can be displayed in the game's UI (ability bars, tooltips, etc.).
	 * Unlike MessageWidgetDataTable which uses UDataTable with row-based lookup, this uses
	 * UDataAsset with array-based storage, providing a simpler configuration approach for
	 * ability data that doesn't require the overhead of Data Table infrastructure.
	 * 
	 * CONFIGURATION:
	 * - EditDefaultsOnly: Can only be set on the class default object (CDO), not on individual instances
	 * - BlueprintReadOnly: Blueprint subclasses can read but not modify this property
	 * - Category="Widget Data": Groups this with MessageWidgetDataTable in the editor details panel
	 * 
	 * DATA STRUCTURE (defined in AbilityInfo.h):
	 * The UAbilityInfo asset contains a TArray<FFoxAbilityInfo> where each element includes:
	 * - AbilityTag: Gameplay tag identifying the ability (e.g., "Abilities.Fire.Fireball")
	 * - InputTag: Tag linking this ability to input bindings (e.g., "InputTag.1" for hotkey slot 1)
	 * - Icon: The ability's icon texture for UI display (shown in ability bars, tooltips)
	 * - BackgroundMaterial: Material for the ability button's background (can indicate cooldown, state, etc.)
	 * 
	 * LOOKUP MECHANISM:
	 * Uses FindAbilityInfoForTag() method (implemented in AbilityInfo.cpp) which iterates through
	 * the AbilityInformation array comparing AbilityTag values until a match is found (O(n) complexity).
	 * This differs from GetDataTableRowByTag() which uses Data Table's hash-based O(1) lookup.
	 * The trade-off: Simpler configuration (no row name requirements) but slower lookups for large datasets.
	 * 
	 * TYPICAL SETUP:
	 * 1. Create a UAbilityInfo Data Asset in the editor (Content Browser → Miscellaneous → Data Asset)
	 * 2. Open the asset and add entries to the AbilityInformation array:
	 *    - Entry 0: AbilityTag="Abilities.Fire.Fireball", InputTag="InputTag.1", Icon=(fireball texture), etc.
	 *    - Entry 1: AbilityTag="Abilities.Lightning.Bolt", InputTag="InputTag.2", Icon=(lightning texture), etc.
	 *    - Entry N: Additional abilities...
	 * 3. Assign this Data Asset to the AbilityInfo property in OverlayWidgetController's class defaults
	 * 4. At runtime, retrieve ability info using: AbilityInfo->FindAbilityInfoForTag(AbilityTag, bLogNotFound)
	 * 
	 * USAGE EXAMPLE:
	 * FGameplayTag FireballTag = FGameplayTag::RequestGameplayTag("Abilities.Fire.Fireball");
	 * FFoxAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(FireballTag, true);
	 * if (Info.AbilityTag.IsValid())
	 * {
	 *     // Info found - display ability icon, set background material, bind to input, etc.
	 *     AbilityIconWidget->SetBrushFromTexture(Info.Icon);
	 *     AbilityButtonWidget->SetBackgroundMaterial(Info.BackgroundMaterial);
	 * }
	 * 
	 * COMPARISON TO MessageWidgetDataTable:
	 * - MessageWidgetDataTable: UDataTable with FUIWidgetRow structure, hash-based O(1) lookup, requires row name convention
	 * - AbilityInfo: UDataAsset with FFoxAbilityInfo array, linear O(n) search, simpler configuration
	 * - MessageWidgetDataTable: Optimized for frequent lookups (UI messages can trigger multiple times per frame)
	 * - AbilityInfo: Acceptable for infrequent lookups (ability bar initialization, ability grants/unlocks)
	 * - Both serve as designer-friendly data repositories avoiding hard-coded ability/message configurations
	 * 
	 * WHY DATA ASSET VS DATA TABLE:
	 * - Data Asset provides simpler workflow for ability designers (just add array elements)
	 * - No need to manage row names or enforce naming conventions
	 * - Ability queries are infrequent (typically only during ability system initialization or ability grants)
	 * - The O(n) search performance is acceptable for small-to-medium ability counts (< 100 abilities)
	 * - If ability count grows significantly, consider migrating to UDataTable for O(1) lookup performance
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	/** Base player controller reference set during initialization via SetWidgetControllerParams(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	/** Base player state reference set during initialization via SetWidgetControllerParams(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	/** Base ability system component reference set during initialization via SetWidgetControllerParams(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Base attribute set reference set during initialization via SetWidgetControllerParams(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	/** Cached Fox-specific player controller, lazily cast from PlayerController via GetFoxPC(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AFoxPlayerController> FoxPlayerController;

	/** Cached Fox-specific player state, lazily cast from PlayerState via GetFoxPS(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AFoxPlayerState> FoxPlayerState;

	/** Cached Fox-specific ability system component, lazily cast from AbilitySystemComponent via GetFoxASC(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UFoxAbilitySystemComponent> FoxAbilitySystemComponent;

	/** Cached Fox-specific attribute set, lazily cast from AttributeSet via GetFoxAS(). */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UFoxAttributeSet> FoxAttributeSet;
	
	/** Returns the cached Fox-specific player controller, casting from base PlayerController if needed. */
	AFoxPlayerController* GetFoxPC();

	/** Returns the cached Fox-specific player state, casting from base PlayerState if needed. */
	AFoxPlayerState* GetFoxPS();

	/** Returns the cached Fox-specific ability system component, casting from base AbilitySystemComponent if needed. */
	UFoxAbilitySystemComponent* GetFoxASC();

	/** Returns the cached Fox-specific attribute set, casting from base AttributeSet if needed. */
	UFoxAttributeSet* GetFoxAS();
};
