// Copyright TryingToMakeGames


#include "UI/HUD/FoxHUD.h"

#include "UI/Widget/FoxUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

UOverlayWidgetController* AFoxHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	// Check if the controller has not been created yet (lazy initialization)
	if (OverlayWidgetController == nullptr)
	{
		/*
		 * - NewObject<UOverlayWidgetController>(): Creates a new UObject-derived instance at runtime (similar to CreateWidget but for non-widget UObjects)
		 * - Template parameter <UOverlayWidgetController>: Specifies the base type to create and the return type of the function
		 * - First parameter (this): Sets the outer object (owner) of the new controller, which is this HUD instance
		 *   The outer relationship is important for object lifecycle management and garbage collection in Unreal's object system
		 * - Second parameter (OverlayWidgetControllerClass): A TSubclassOf<UOverlayWidgetController> that references the specific class to instantiate
		 *   This allows Blueprint-defined controller classes to be created, providing flexibility to extend controller functionality in Blueprints
		 * - The newly created controller is stored in the OverlayWidgetController member variable for reuse in subsequent calls
		 */
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		
		/*
		 * - Injects the dependencies (WCParams) into the controller
		 * - WCParams is the input parameter to the function we are inside of.
		 * - WCParams is a FWidgetControllerParams (defined in FoxWidgetController.h) struct containing the four core gameplay 
		 *   system references (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet) 
		 * - SetWidgetControllerParams() stores these references as member variables in the controller, making them accessible
		 *   for all subsequent operations like binding to attribute changes and broadcasting values
		 * - This follows dependency injection pattern, keeping the controller decoupled from how it obtains these references
		*/
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		
		// Calls a function on the controller. See the comments there for more details
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	// Return the cached controller instance (either newly created or previously initialized)
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AFoxHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// Check if the controller has not been created yet (lazy initialization)
	if (AttributeMenuWidgetController == nullptr)
	{
		/*
		 * - NewObject<UAttributeMenuWidgetController>(): Creates a new UObject-derived instance at runtime (similar to CreateWidget but for non-widget UObjects)
		 * - Template parameter <UOverlayWidgetController>: Specifies the base type to create and the return type of the function
		 * - First parameter (this): Sets the outer object (owner) of the new controller, which is this HUD instance
		 *   The outer relationship is important for object lifecycle management and garbage collection in Unreal's object system
		 * - Second parameter (OverlayWidgetControllerClass): A TSubclassOf<UOverlayWidgetController> that references the specific class to instantiate
		 *   This allows Blueprint-defined controller classes to be created, providing flexibility to extend controller functionality in Blueprints
		 * - The newly created controller is stored in the OverlayWidgetController member variable for reuse in subsequent calls
		 */
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		
		/*
		 * - Injects the dependencies (WCParams) into the controller
		 * - WCParams is the input parameter to the function we are inside of.
		 * - WCParams is a FWidgetControllerParams (defined in FoxWidgetController.h) struct containing the four core gameplay 
		 *   system references (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet) 
		 * - SetWidgetControllerParams() stores these references as member variables in the controller, making them accessible
		 *   for all subsequent operations like binding to attribute changes and broadcasting values
		 * - This follows dependency injection pattern, keeping the controller decoupled from how it obtains these references
		*/
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		
		// Calls a function on the controller. See the comments there for more details
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

void AFoxHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_FoxHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_FoxHUD"));
	
	/*
	 * These two lines are responsible for creating and displaying the overlay UI widget:
	 * 
	 * Line 1: UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	 * - CreateWidget<UUserWidget>(): This is a templated function that creates a new UMG widget instance
	 * - Template parameter <UUserWidget>: Specifies the type of widget to create (in this case, base UUserWidget class)
	 * - GetWorld(): Retrieves the current world context, which is required for widget creation as widgets need to know which game world they belong to
	 * - OverlayWidgetClass: A TSubclassOf<UFoxUserWidget> member variable that holds a reference to the Blueprint or C++ class to instantiate
	 *   This is set via EditAnywhere in the editor and determines which specific widget class will be created
	 * - The function returns a pointer to the newly created widget instance, which is stored in the local Widget variable
	 * 
	 * Line 2: Widget->AddToViewport();
	 * - AddToViewport(): This function adds the created widget to the game's viewport, making it visible on screen
	 * - Without this call, the widget would exist in memory but wouldn't be rendered
	 * - By default, this adds the widget at Z-order 0, meaning it appears on top of the game world but can be behind other UI elements with higher Z-orders
	 * - This is the final step to make the UI visible to the player
	 * 
	 * Together, these lines instantiate the overlay widget (typically defined in Blueprints and referenced via OverlayWidgetClass)
	 * and display it on the player's screen when the HUD begins play.
	 */
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UFoxUserWidget>(Widget);
	
	/*
	 * These two lines create and configure the widget controller that manages data flow between gameplay systems and UI:
	 * 
	 * Line 1: const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	 * - Creates a parameter struct that bundles together the four core gameplay system references needed by the widget controller
	 * - PC (APlayerController*): The player controller that handles player input and owns the viewport
	 * - PS (APlayerState*): The player state that persists player-specific data and owns the AbilitySystemComponent in this architecture
	 * - ASC (UAbilitySystemComponent*): The Gameplay Ability System component that manages abilities, effects, and attributes
	 * - AS (UAttributeSet*): The attribute set containing gameplay attributes (Health, MaxHealth, Mana, MaxMana, etc.)
	 * - This struct pattern provides a clean way to pass multiple related parameters as a single unit, making the code more maintainable
	 *   and easier to extend if additional parameters are needed in the future
	 * 
	 * Line 2: UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	 * - Retrieves or creates the OverlayWidgetController using the parameter struct
	 * - GetOverlayWidgetController() implements lazy initialization: it creates the controller only on first call and caches it for subsequent calls
	 * - Inside GetOverlayWidgetController(), the controller calls SetWidgetControllerParams() to store these references internally,
	 *   then calls BindCallbacksToDependencies() to register callbacks that listen for attribute changes from the AbilitySystemComponent
	 * - The returned controller acts as an intermediary layer between the raw gameplay systems (ASC, AttributeSet) and the UI widget,
	 *   translating attribute changes into UI-friendly events and providing a clean separation of concerns
	 * - This architecture follows the MVC pattern where the widget is the View, the controller is the Controller,
	 *   and the gameplay systems (PlayerState, ASC, AttributeSet) are the Model
	 */
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
	
	/*
	 * These three lines complete the initialization chain by connecting the widget controller to the widget,
	 * populating initial attribute data, and displaying the UI:
	 * 
	 * Line 1: OverlayWidget->SetWidgetController(WidgetController);
	 * - Establishes the connection between the UFoxUserWidget (OverlayWidget) and its UOverlayWidgetController (WidgetController)
	 * - This call stores the controller reference in the widget so it can listen to the controller's broadcast delegates
	 * - After this connection, the widget can bind to events like OnHealthChanged, OnMaxHealthChanged, etc.
	 *   that the controller will broadcast when attribute values change
	 * - This completes the MVC wiring: the View (widget) now knows about its Controller (widget controller),
	 *   and the Controller already knows about the Model (ASC, AttributeSet) from the earlier SetWidgetControllerParams() call
	 * 
	 * Line 2: WidgetController->BroadcastInitialValues();
	 * - Triggers the widget controller to immediately broadcast the current values of all attributes (Health, MaxHealth, Mana, MaxMana)
	 * - This is essential for proper initialization because it populates the UI with correct values the moment it appears,
	 *   rather than waiting for the first attribute change to occur
	 * - Without this call, the UI would display default/uninitialized values (often zeros) until gameplay events modified the attributes
	 * - The broadcast delegates fire synchronously, causing the widget's bound callbacks to execute and update UI elements
	 *   with the actual current attribute values from the AttributeSet
	 * - This ensures the player sees accurate information immediately when the HUD appears, providing a polished user experience
	 * 
	 * Line 3: Widget->AddToViewport();
	 * - Adds the fully initialized and data-populated widget to the game's viewport, making it visible on screen
	 * - At this point, the widget has its controller reference set and has received initial attribute values,
	 *   so it appears with correct data from the first frame
	 * - The order matters: SetWidgetController() → BroadcastInitialValues() → AddToViewport() ensures the widget
	 *   is fully configured before becoming visible, preventing any flicker or flash of incorrect values
	 * - This completes the entire overlay initialization sequence, resulting in a fully functional,
	 *   data-driven UI that responds to gameplay attribute changes in real-time
	 */
	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}

USpellMenuWidgetController* AFoxHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// Check if the controller has not been created yet (lazy initialization)
	if (SpellMenuWidgetController == nullptr)
	{
		/*
		 * - NewObject<USpellMenuWidgetController>(): Creates a new UObject-derived instance at runtime for managing spell menu UI data
		 * - Template parameter <USpellMenuWidgetController>: Specifies both the base type to create and the return type
		 * - First parameter (this): Sets the outer object (owner) to this HUD instance, establishing ownership for garbage collection
		 *   The outer relationship ensures the controller's lifetime is tied to the HUD's lifetime in Unreal's object system
		 * - Second parameter (SpellMenuWidgetControllerClass): A TSubclassOf<USpellMenuWidgetController> reference to the specific class to instantiate
		 *   This allows Blueprint-defined controller classes to be created, enabling designers to extend spell menu functionality
		 *   without modifying C++ code
		 */
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
			
		/*
		 * - Injects the dependencies (WCParams) into the controller
		 * - WCParams is the input parameter to the function we are inside of.
		 * - WCParams is a FWidgetControllerParams (defined in FoxWidgetController.h) struct containing the four core gameplay 
		 *   system references (PlayerController, PlayerState, AbilitySystemComponent, AttributeSet) 
		 * - SetWidgetControllerParams() stores these references as member variables in the controller, making them accessible
		 *   for all subsequent operations like binding to attribute changes and broadcasting values
		 * - This follows dependency injection pattern, keeping the controller decoupled from how it obtains these references
		*/
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
			
		// Calls a function on the controller. See the comments there for more details
		SpellMenuWidgetController->BindCallbacksToDependencies();
	}
	// Return the cached controller instance (either newly created or previously initialized)
	return SpellMenuWidgetController;
}
