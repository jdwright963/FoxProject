// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "FoxInputConfig.h"
#include "FoxInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Binds ability input actions to callback functions for pressed, released, and held events.
	 * 
	 * WHAT IS A TEMPLATE?
	 * The "template" line allows this function to work with any class type and any function types.
	 * Instead of writing separate versions of this function for each possible class or function signature,
	 * templates let the compiler generate the appropriate version automatically based on what you pass in.
	 * Think of it like a blueprint that adapts to whatever types you use when calling the function.
	 * 
	 * typename is essentially a placeholder that tells the compiler: "I don't know what this type is yet, but when 
	 * someone calls this function, treat this name as a data type." Same for class in template line
	 * 
	 * TEMPLATE PARAMETERS:
	 * - UserClass: The type of object that owns the callback functions (e.g., AFoxPlayerController, AMyCharacter)
	 * - PressedFuncType: The type/signature of the function to call when an ability input is first pressed
	 * - ReleasedFuncType: The type/signature of the function to call when an ability input is released
	 * - HeldFuncType: The type/signature of the function to call when an ability input is held down
	 * 
	 * FUNCTION PARAMETERS:
	 * @param InputConfig - Data asset containing the array of ability input actions with their associated gameplay tags
	 * @param Object - Pointer to the object instance that contains the callback functions (usually 'this')
	 * @param PressedFunc - Member function pointer to call when input is pressed (e.g., &AMyClass::OnAbilityPressed)
	 * @param ReleasedFunc - Member function pointer to call when input is released (e.g., &AMyClass::OnAbilityReleased)
	 * @param HeldFunc - Member function pointer to call while input is held (e.g., &AMyClass::OnAbilityHeld)
	 * 
	 * HOW IT WORKS:
	 * This function iterates through all ability input actions defined in the InputConfig data asset.
	 * For each input action, it binds the appropriate callback functions (pressed/released/held) to the
	 * Enhanced Input system, allowing your character or controller to respond to ability inputs.
	 * This creates a flexible system where input actions are data-driven via the InputConfig asset
	 * and can trigger gameplay abilities through the Gameplay Ability System.
	 * 
	 * EXAMPLE USAGE:
	 * InputComponent->BindAbilityActions(AbilityInputConfig, this, 
	 *     &ThisClass::AbilityInputPressed, 
	 *     &ThisClass::AbilityInputReleased, 
	 *     &ThisClass::AbilityInputHeld);
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
	void BindAbilityActions(const UFoxInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc);
};

/**
 * Implementation of the BindAbilityActions function
 * 
 * WHY TEMPLATE FUNCTIONS ARE IMPLEMENTED IN HEADER FILES (.h) AND NOT SOURCE FILES (.cpp)
 * 
 * Template functions MUST be implemented in header files (or included in headers) because of how C++ compilation works.
 * Here's why:
 * 
 * THE COMPILATION PROCESS:
 * C++ compiles each .cpp file independently into object files before linking them together. When the compiler
 * encounters a template function call in a .cpp file, it needs to see the FULL IMPLEMENTATION to generate
 * the specific version of that function for the types being used (called "template instantiation").
 * 
 * THE PROBLEM WITH .cpp IMPLEMENTATION:
 * If you put a template implementation in a .cpp file:
 * 1. That .cpp file compiles and creates an object file with NO concrete functions (just the template definition)
 * 2. When OTHER .cpp files try to use the template with specific types, they can't see the implementation
 * 3. The compiler can't instantiate the template because the implementation is in a different compilation unit
 * 4. This results in "unresolved external symbol" linker errors
 * 
 * THE SOLUTION - HEADER IMPLEMENTATION:
 * By implementing templates in header files:
 * 1. Every .cpp file that includes the header gets a COPY of the template implementation
 * 2. The compiler can instantiate the template with whatever types that .cpp file uses
 * 3. Each compilation unit generates its own versions of the template function as needed
 * 4. The linker removes duplicate instantiations, keeping only one copy of each unique type combination
 * 
 * EXAMPLE:
 * If FoxPlayerController.cpp calls BindAbilityActions with specific function pointer types, the compiler
 * needs to see this implementation to create a version of BindAbilityActions that works with those exact types.
 * If the implementation was in FoxInputComponent.cpp, the compiler couldn't generate that specialized version.
 * 
 * ALTERNATIVES (Advanced):
 * - Explicit template instantiation: Manually instantiate all needed versions in the .cpp (rarely practical)
 * - Separate .inl files: Some projects use .inl (inline) files for template implementations, included at the end of headers
 * 
 * BOTTOM LINE:
 * Template implementations in headers are not just convention - they're a requirement of the C++ compilation model.
 * The compiler must see both the template declaration AND implementation to generate type-specific code.
 */
template <class UserClass, typename PressedFuncType, typename ReleasedFuncType, typename HeldFuncType>
void UFoxInputComponent::BindAbilityActions(const UFoxInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, HeldFuncType HeldFunc)
{
	// If this object is null crash the program
	check(InputConfig);
	
	// Loop through the items in the AbilityInputActions array of the data asset
	for (const FFoxInputAction& Action : InputConfig->AbilityInputActions)
	{
		// Check if the current element of the array has a nonnull InputAction and the tag is valid
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// Check if the function pointer is not null
			if (PressedFunc)
			{
				// ETriggerEvent::Started - Fires on the first frame when input begins. See the HeldFunc line for an 
				// explanation of the rest of this line.
				BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag);
			}
			if (ReleasedFunc)
			{
				// ETriggerEvent::Completed - Fires when input is released. See the HeldFunc line for an explanation
				// of the rest of this line.
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag);
			}
			if (HeldFunc)
			{
				/**
				 * BINDS THE INPUT ACTION TO THE HELD CALLBACK FUNCTION
				 * 
				 * BindAction is an Enhanced Input Component function that creates a binding between an input action
				 * and a callback function. This establishes the connection that makes the input system respond to player input.
				 * 
				 * PARAMETERS BREAKDOWN:
				 * 
				 * 1. Action.InputAction (const UInputAction*)
				 *    - The input action asset that defines WHAT input we're listening for (e.g., a specific key press, mouse click, gamepad button)
				 *    - This comes from the InputConfig data asset and represents a specific ability input
				 *    - Examples: "Jump", "Primary Attack", "Special Ability 1"
				 * 
				 * 2. ETriggerEvent::Triggered
				 *    - Specifies WHEN the callback should fire during the input lifecycle
				 *    - ETriggerEvent::Triggered fires continuously every frame while the input is held down (after initial trigger conditions are met)
				 *    - Other options include: Started (first frame pressed), Ongoing (every frame while active), Completed (when released), Canceled
				 *    - For HeldFunc, we use Triggered because we want continuous updates while the player holds the input
				 * 
				 * 3. Object (UserClass*)
				 *    - Pointer to the object instance that OWNS the callback function we want to call
				 *    - Usually 'this' when called from a Controller or Character class
				 *    - The binding needs to know which object instance to call the member function on
				 *    - Example: if Object is AFoxPlayerController*, it knows to call the function on that specific controller instance
				 * 
				 * 4. HeldFunc (HeldFuncType - function pointer)
				 *    - The actual member function pointer that will be CALLED when the input event fires
				 *    - This is passed in as a template parameter, allowing any compatible function signature
				 *    - Typically expects a signature like: void YourClass::FunctionName(const FGameplayTag&)
				 *    - Example: &AFoxPlayerController::AbilityInputHeld
				 * 
				 * 5. Action.InputTag (FGameplayTag)
				 *    - The gameplay tag associated with this input action, passed as an argument to the callback function
				 *    - This allows the callback to know WHICH ability input was triggered
				 *    - The tag is used by the Gameplay Ability System to identify and activate specific abilities
				 *    - Example: if the tag is "InputTag.Ability.Fire", the callback can activate the fire ability
				 * 
				 * WHAT HAPPENS WHEN THIS EXECUTES:
				 * When the player presses and holds the input associated with Action.InputAction, the Enhanced Input System
				 * will call Object->HeldFunc(Action.InputTag) every frame while the input remains held. This allows
				 * the Gameplay Ability System to continuously update held abilities (like charging a shot or channeling a spell).
				 * 
				 * EXAMPLE FLOW:
				 * 1. Player holds the "Q" key (mapped to "Ability1" input action with tag "InputTag.Ability.Q")
				 * 2. Every frame while held, Enhanced Input System calls: PlayerController->AbilityInputHeld(FGameplayTag("InputTag.Ability.Q"))
				 * 3. The AbilityInputHeld function can then handle the ability logic (e.g., increase charge level, maintain channel)
				 */
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, HeldFunc, Action.InputTag);
			}
		}
	}
}
