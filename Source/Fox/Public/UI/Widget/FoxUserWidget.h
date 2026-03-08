// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FoxUserWidget.generated.h"

/**
 * UFoxUserWidget - Base class for all custom UI widgets in the Fox project.
 * 
 * This class extends UUserWidget, which is Unreal Engine's core class for creating user interface elements
 * through the UMG (Unreal Motion Graphics) system. UUserWidget provides the foundation for building interactive
 * UI widgets that can be designed visually in the UMG Designer and programmed in both C++ and Blueprints.
 * 
 * What is UUserWidget?
 * UUserWidget is Unreal's base class for all UMG widgets, serving as a container and controller for UI elements.
 * It handles widget lifecycle (construction, initialization, destruction), manages child widget hierarchies,
 * provides animation support, handles input events, and can be added to the viewport for display. Widgets created
 * from UUserWidget can contain other widgets (buttons, text blocks, images, etc.) and define their visual layout
 * and interactive behavior. They are typically created as Blueprint classes that inherit from UUserWidget and
 * designed in the UMG Visual Designer, though they can also be created entirely in C++.
 * 
 * Purpose of UFoxUserWidget:
 * This custom class implements the Model-View-Controller (MVC) pattern for UI management by introducing the
 * concept of a WidgetController. Instead of having UI widgets directly access game data (PlayerState, 
 * AbilitySystemComponent, etc.), UFoxUserWidget separates concerns by:
 * 
 * - Acting as the View: Responsible only for displaying information and handling user input visually
 * - Delegating data management to a WidgetController: The controller (stored in WidgetController property)
 *   serves as an intermediary that fetches data from game systems and provides it to the widget in a 
 *   presentation-ready format
 * - Supporting Blueprint extensibility: The WidgetControllerSet() event allows Blueprint subclasses to
 *   respond when the controller is assigned, enabling them to bind to controller delegates and update UI
 * 
 * This architecture promotes:
 * - Clean separation between UI presentation and game logic
 * - Reusable controllers that can work with multiple widget types
 * - Easier testing and maintenance by isolating dependencies
 * - Blueprint-friendly workflow where designers can work with controllers without touching game systems directly
 * 
 * Usage Example:
 * 1. Create a Blueprint widget class based on UFoxUserWidget
 * 2. Design the visual layout in UMG Designer
 * 3. In BeginPlay or equivalent, create an appropriate WidgetController (e.g., UOverlayWidgetController)
 * 4. Call SetWidgetController() to assign the controller to the widget
 * 5. Implement WidgetControllerSet() in Blueprint to bind to controller's data broadcasts
 * 6. The controller handles all communication with PlayerState, AbilitySystemComponent, and other game systems
 * 
 * @see UFoxWidgetController - Base controller class that manages widget data and dependencies
 * @see UOverlayWidgetController - Example controller implementation for overlay HUD widgets
 * @see AFoxHUD::InitOverlay() - Example of creating and initializing a UFoxUserWidget with its controller
 */

UCLASS()
class FOX_API UFoxUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
protected:

	/**
	 * WidgetControllerSet - Blueprint-implementable event that is called when the widget controller is assigned to this widget.
	 * 
	 * What is a BlueprintImplementableEvent?
	 * A BlueprintImplementableEvent is a special type of UFUNCTION that can only be implemented in Blueprint classes,
	 * not in C++. When declared in a C++ class, it creates a Blueprint event node that subclasses can override and
	 * implement with Blueprint visual scripting. The C++ code can call this function, and the call will execute
	 * whatever Blueprint logic has been implemented in the subclass. If no Blueprint implementation exists, the
	 * function call safely does nothing.
	 * 
	 * Purpose and Role in MVC Pattern:
	 * This event serves as the critical initialization hook that allows Blueprint widgets to complete the MVC
	 * connection established by SetWidgetController(). When SetWidgetController() assigns a controller to the
	 * WidgetController property, it immediately calls this WidgetControllerSet() event, giving Blueprint subclasses
	 * the opportunity to:
	 * 
	 * 1. Access the newly assigned WidgetController reference
	 * 2. Cast it to the appropriate controller type (e.g., UOverlayWidgetController)
	 * 3. Bind to the controller's delegate broadcasts (OnHealthChanged, OnMaxHealthChanged, etc.)
	 * 4. Set up any additional initialization logic specific to that widget
	 * 
	 * Call Sequence in the MVC Initialization Flow:
	 * This event fits into the larger widget initialization sequence as follows:
	 * 
	 * 1. AFoxHUD::InitOverlay() creates the widget: CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass)
	 * 2. InitOverlay() creates the controller: GetOverlayWidgetController(WidgetControllerParams)
	 * 3. InitOverlay() connects them: OverlayWidget->SetWidgetController(WidgetController)
	 *    └─> SetWidgetController() stores the controller reference in WidgetController property
	 *        └─> SetWidgetController() calls WidgetControllerSet() ← YOU ARE HERE
	 *            └─> Blueprint implementation binds to controller's delegates (OnHealthChanged, etc.)
	 * 4. InitOverlay() broadcasts initial values: WidgetController->BroadcastInitialValues()
	 *    └─> Triggers all bound delegate callbacks, populating UI with current attribute values
	 * 5. InitOverlay() displays the widget: Widget->AddToViewport()
	 * 
	 * Typical Blueprint Implementation Pattern:
	 * In a Blueprint widget class derived from UFoxUserWidget (e.g., WBP_Overlay), you would implement this event like:
	 * 
	 * Event WidgetControllerSet
	 *   ├─> Get WidgetController
	 *   ├─> Cast to OverlayWidgetController
	 *   ├─> Bind Event to OnHealthChanged
	 *   │     └─> Update HealthProgressBar with new value
	 *   ├─> Bind Event to OnMaxHealthChanged
	 *   │     └─> Update HealthText with max value
	 *   ├─> Bind Event to OnManaChanged
	 *   │     └─> Update ManaProgressBar with new value
	 *   └─> Bind Event to OnMaxManaChanged
	 *         └─> Update ManaText with max value
	 * 
	 * Why This Pattern is Important:
	 * - Separation of Concerns: Blueprint designers can bind UI updates without knowing how the controller fetches data
	 * - Reusability: The same controller can be used with different widget implementations
	 * - Type Safety: Blueprint casting ensures you're working with the correct controller type for your widget
	 * - Initialization Guarantee: By the time this event fires, WidgetController is guaranteed to be valid and assigned
	 * - Clean Architecture: Follows MVC by keeping the View (widget) dependent on the Controller (widget controller)
	 *   but not directly on the Model (PlayerState, AbilitySystemComponent)
	 * 
	 * Example Use Case:
	 * When a player's health changes during gameplay:
	 * 1. AbilitySystemComponent modifies the Health attribute in the AttributeSet
	 * 2. Controller's bound callback receives the attribute change notification
	 * 3. Controller broadcasts OnHealthChanged delegate with the new value
	 * 4. Widget's Blueprint callback (bound during WidgetControllerSet) receives the new value
	 * 5. Widget updates the health bar and text UI elements to reflect the new health value
	 * 
	 * @note This function has no C++ implementation and must be overridden in Blueprint to have any effect
	 * @note The function is called automatically by SetWidgetController() after assigning the WidgetController property
	 * @note Always check if WidgetController is valid and cast to the expected controller type in Blueprint implementations
	 * @see SetWidgetController() - The function that calls this event after assigning the controller
	 * @see UOverlayWidgetController - Example controller class that provides delegates for binding in this event
	 * @see AFoxHUD::InitOverlay() - Example of the complete initialization sequence that leads to this event being called
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
