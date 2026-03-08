// Copyright TryingToMakeGames


#include "Player/FoxPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "FoxGameplayTags.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Input/FoxInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "ProfilingDebugging/CookStats.h"
#include "UI/Widget/DamageTextComponent.h"

AFoxPlayerController::AFoxPlayerController()
{
	bReplicates = true;
	
	// This is related to auto run and will need to be removed
	// Creates the spline component which will appear in the blueprint. 
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AFoxPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	
	// This relates to auto running and needs to be removed
	AutoRun();
}

// '_Implementation' is auto generated when generating definitions of RPCs
void AFoxPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	/*
	 Checks if TargetCharacter input parameter and the DamageTextComponentClass member variable are null.
	 In addition to checking for null IsValid() checks if the pointer is pending kill which is true when Destroy()
	 has been called on an Actor, which is not applicable for DamageTextComponentClass. `IsLocalController` checks 
	 if this class is the local player controller, so that we do not show damage numbers on the server, only on the 
	 client who is the source of the damage
	*/
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		
	/*
	   Creates a new instance of UDamageTextComponent using Unreal's object construction system.
	   
	   Breaking down this line:
	   1. NewObject<UDamageTextComponent>: Template function that constructs a new UObject-derived instance.
	      The template parameter specifies we want to create a UDamageTextComponent object.
	   
	   2. TargetCharacter (first parameter - Outer): The owner/outer object for this component. Passing TargetCharacter
		  makes the target character the outer, which is important for object lifetime management and
	      replication. The component will be destroyed when the target character is destroyed.
	   
	   3. DamageTextComponentClass (second parameter - Class): A TSubclassOf<UDamageTextComponent> that specifies
	      the exact class to instantiate. This allows us to use a Blueprint-derived class set in the editor
	      rather than hardcoding the base C++ class, enabling designers to customize the damage text widget
	      appearance and behavior without code changes.
	 */
	UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);

	/*
	   Registers the newly created component with the engine's component management system.
	   
	   RegisterComponent() performs several critical initialization steps:
	   - Adds the component to the engine's component registry for ticking and rendering
	   - Initializes the component's scene proxy if it's a scene component
	   - Calls the component's OnRegister() function for custom initialization logic
	   - Makes the component eligible for replication if configured
	   
	   Without registration, the component exists in memory but won't be updated, rendered, or properly
	   integrated into the game world. This is necessary because we created the component dynamically
	   at runtime using NewObject rather than in the constructor with CreateDefaultSubobject.
	 */
	DamageText->RegisterComponent();

	/*
	   Attaches the damage text component to the target character's root component.
	   
	   Breaking down this line:
	   1. AttachToComponent(): Method that establishes a parent-child relationship between components in the
	      scene hierarchy. The damage text will now follow the parent's transforms.
	   
	   2. TargetCharacter->GetRootComponent() (first parameter - Parent): Retrieves the root component of the
	      character taking damage. The root component is typically a capsule or mesh component that represents
	      the character's location and orientation in the world. By attaching to this, the damage text will
	      move with the character as they move around.
	   
	   3. FAttachmentTransformRules::KeepRelativeTransform (second parameter - AttachmentRules): Specifies how
	      the component's transform should be calculated after attachment. KeepRelativeTransform preserves
	      the component's current location, rotation, and scale relative to its new parent, maintaining its
	      offset from the character rather than snapping to the parent's pivot point.
	 */
	DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	/*
	   Detaches the damage text component from its parent while preserving its current world-space transform.
	   
	   Breaking down this line:
	   1. DetachFromComponent(): Breaks the parent-child attachment established by AttachToComponent, making
	      the component independent in the scene hierarchy. This allows the damage text to remain stationary
	      in world space even as the character moves.
	   
	   2. FDetachmentTransformRules::KeepWorldTransform (parameter - DetachmentRules): Specifies that the
	      component should maintain its current world-space location, rotation, and scale after detachment.
	      This means the damage text will stay exactly where it was positioned relative to the world when
	      it was attached, rather than jumping to a new location. This is crucial for damage numbers that
	      should float at the point of impact while the character continues moving.
	   
	   Note: The pattern of attaching then immediately detaching is used to easily position the component
	   relative to the character's root, then freeze it at that world position for the duration of its
	   display animation.
	 */
	DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		/*
		   Sets the damage amount to be displayed by the damage text component.
		   
		   Breaking down this line:
		   1. SetDamageText(): Custom method defined in UDamageTextComponent that configures the widget to display
		      the damage value. This likely converts the float to a formatted string and updates the widget's
		      text element.
		   
		   2. DamageAmount (parameter): The float value representing how much damage was dealt, passed into this
		      RPC from the server. This is the actual gameplay-significant number that will be shown to the player,
		      such as "127.5" or "1000", providing visual feedback about the effectiveness of their attacks.
		*/
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

// This relates to auto running and needs to be removed
void AFoxPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);
		
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination < AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}	
}

// We do not want this cursor trace that highlights actors in the final game this all needs to be removed
void AFoxPlayerController::CursorTrace()
{
	
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_CursorTrace))
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->UnHighlightActor();
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.IsValidBlockingHit()) return;
	
	/*
	   Stores the actor from the previous frame's cursor trace into LastActor for comparison.

	   This assignment preserves the actor that was under the cursor in the previous frame, allowing us to detect
	   when the cursor has moved from one actor to another (or from an actor to empty space, or vice versa). By
	   comparing LastActor to the newly retrieved ThisActor, we can determine whether highlighting state needs to
	   change, unhighlighting the previously hovered actor and/or highlighting the newly hovered actor.
	 */
	LastActor = ThisActor;

	/*
	   Retrieves the actor hit by the current frame's cursor trace and stores it in ThisActor.

	   Breaking down this line:
	   1. CursorHit.GetActor(): Extracts the AActor pointer from the FHitResult struct populated by the
	      GetHitResultUnderCursor call above. Returns the actor that was hit by the trace, or nullptr if no actor
	      was hit.

	   2. ThisActor assignment: Stores the actor in a TScriptInterface<IEnemyInterface> wrapper rather than a raw
	      pointer. This differs from the tutorial code which used a raw pointer. The TScriptInterface wrapper provides:
	        - Type safety by ensuring the actor implements IEnemyInterface
	        - Automatic interface casting without explicit Cast<>() calls
	        - Blueprint compatibility for interface method calls
	        - Null safety through built-in validity checks

	   3. Usage context: If ThisActor contains a valid pointer (non-null and implements IEnemyInterface), a click
	      should trigger targeting behavior for attacking/interacting with that enemy. If ThisActor is null (cursor
	      over empty space or a non-enemy actor), a click should trigger auto-running behavior to move the character
	      to the clicked location. This auto-run distinction will be removed in future refactoring.
	 */
	ThisActor = CursorHit.GetActor();
	
	/*
	   Line trace from cursor. There are several scenarios:
	    A. LastActor is null && ThisActor is null
	        - Do nothing
	    B. LastActor is null && ThisActor is valid
	        - Highlight ThisActor
	    C. LastActor is valid && ThisActor is null
	        - UnHighlight LastActor
	    D. Both actors are valid, but LastActor != ThisActor
	        - UnHighlight LastActor, and Highlight ThisActor
	    E. Both actors are valid, and are the same actor
	        - Do nothing
	 */
	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->HighlightActor();
	}
}

void AFoxPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	/*
	   Checks if input should be blocked based on gameplay tags applied to the Ability System Component.

	   Breaking down this line:
	   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it if not yet cached).
	      Returns nullptr if the controlled pawn doesn't have an ASC. This null check is necessary to prevent crashes
	      when attempting to call methods on a non-existent ASC.

	   2. &&: Logical AND operator with short-circuit evaluation. If GetASC() returns nullptr (false), the right side
	      of the expression is never evaluated, preventing a null pointer dereference when calling HasMatchingGameplayTag.

	   3. GetASC()->HasMatchingGameplayTag(): Method from UAbilitySystemComponent that checks if any gameplay tags
	      currently applied to this ASC match the provided tag. Returns true if a matching tag is found, false otherwise.
	      Gameplay tags on the ASC can be added by active gameplay effects, active abilities, or directly through code,
	      allowing dynamic control over which systems can process input.

	   4. FFoxGameplayTags::Get().Player_Block_InputPressed: Retrieves the native C++ gameplay tag that indicates input
	      pressed events should be blocked. When this tag is present on the ASC (e.g., applied by an ability like GA_Electrocute,
	      a crowd control effect, a cutscene, or UI state), it prevents the player from activating new abilities or
	      triggering input-related actions. This provides a centralized way to disable input processing without
	      modifying every input handling function.

	   If both conditions are true (ASC exists AND the blocking tag is present), the function returns early without
	   forwarding the input to the ability system, effectively ignoring the player's input press.
	 */
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputPressed))
	{
		// Return early without processing the input pressed event, preventing ability activation while input is blocked
		return;
	}
	
	// Checks if the InputTag input parameter exactly matches the native C++ gameplay tag defined for the left mouse
	// button
	if (InputTag.MatchesTagExact(FFoxGameplayTags::Get().InputTag_LMB))
	{
		// The following 2 lines are related to auto running and will need to be removed
		// If ThisActor is not a null pointer then bTargeting is assigned true (a click should cause targeting). 
        // If ThisActor is a null pointer then it is false (a click should cause auto running)
        bTargeting = ThisActor ? true : false;
        
        // Set to false because we do not know if the click will be a short click or long one until it is released and this
        // function is only for click events 
        bAutoRunning = false;
	}
	
	/*
	   Forwards the pressed input event to the Ability System Component for processing.

	   Breaking down this line:
	   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it
	      if not yet cached). Returns nullptr if the controlled pawn doesn't have an ASC.

	   2. if (GetASC()): Null pointer check to ensure the ASC exists before attempting to call methods on it.
	      This prevents crashes if called before the pawn is spawned or if the pawn doesn't implement the
	      IAbilitySystemInterface.

	   3. GetASC()->AbilityInputTagPressed(InputTag): Calls the custom method on our UFoxAbilitySystemComponent
	      that handles pressed input events. This method iterates through all abilities granted to the player
	      and attempts to activate any abilities that are bound to the provided InputTag. For example, if
	      InputTag matches an ability's activation tag, that ability will be triggered to start executing.

	   4. InputTag parameter: The FGameplayTag identifying which input was pressed (e.g., InputTag_LMB for
	      left mouse button, InputTag_1 for ability hotkey 1, etc.). The ASC uses this tag to determine
	      which abilities should respond to this input event and attempt activation.

	   This forwarding mechanism allows the player controller to act as an input router, translating
	   raw input actions into gameplay ability activations through the Gameplay Ability System.
	 */
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AFoxPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	/*
	   Checks if input released events should be blocked based on gameplay tags applied to the Ability System Component.

	   Breaking down this line:
	   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it if not yet cached).
	      Returns nullptr if the controlled pawn doesn't have an ASC. This null check is necessary to prevent crashes
	      when attempting to call methods on a non-existent ASC.

	   2. &&: Logical AND operator with short-circuit evaluation. If GetASC() returns nullptr (false), the right side
	      of the expression is never evaluated, preventing a null pointer dereference when calling HasMatchingGameplayTag.

	   3. GetASC()->HasMatchingGameplayTag(): Method from UAbilitySystemComponent that checks if any gameplay tags
	      currently applied to this ASC match the provided tag. Returns true if a matching tag is found, false otherwise.
	      Gameplay tags on the ASC can be added by active gameplay effects, active abilities, or directly through code,
	      allowing dynamic control over which systems can process input.

	   4. FFoxGameplayTags::Get().Player_Block_InputReleased: Retrieves the native C++ gameplay tag that indicates input
	      released events should be blocked. When this tag is present on the ASC (e.g., applied by an ability like GA_Electrocute,
	      a crowd control effect, a cutscene, or UI state), it prevents the player from triggering input release-related 
	      actions. This provides a centralized way to disable input released processing without modifying every input handling function.

	   If both conditions are true (ASC exists AND the blocking tag is present), the function returns early without
	   forwarding the input to the ability system, effectively ignoring the player's input release.
	 */
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputReleased))
	{
		// Return early without processing the input released event, preventing ability release callbacks while input is blocked
		return;
	}
	
	// Checks if the InputTag input parameter is not the native C++ gameplay tag defined for the left mouse
	// button
	if (!InputTag.MatchesTagExact(FFoxGameplayTags::Get().InputTag_LMB))
	{
		// Checks if the ASC is not a null pointer
		if (GetASC())
		{
			/*
			   Forwards the held input event to the Ability System Component for processing.
			   
			   Breaking down this line:
			   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it
			      if not yet cached). We've already verified this is non-null in the if statement above.
			   
			   2. ->AbilityInputTagReleased(InputTag): Calls the custom method on our UFoxAbilitySystemComponent that
			      handles released input events. This method iterates through all activated abilities and calls
			      their input released callbacks, allowing abilities to respond to released input (e.g., charging
			      an attack, continuous channeling, or maintaining a blocking stance).
			   
			   3. InputTag parameter: The FGameplayTag identifying which input is being held (e.g., an ability
			      hotkey like InputTag_1, InputTag_2, etc.). The ASC uses this tag to determine which abilities
			      should respond to this held input event.
			   
			   Note: Left mouse button (LMB) input is intentionally excluded from this forwarding (checked above)
			   because LMB has special handling for targeting and auto-running behavior in this controller. This will be
			   removed.
			 */
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		// We just tried to activate the ability (or the ASC was null pointer) so we can return early
		return;
	}
	
	// Checks if the ASC is not a null pointer. The contents of this if statement are outside any of the other if
	// statements because we want to let the ASC know that we have released the input regardless
	if (GetASC())
	{
		/*
		   Forwards the left mouse button held input to the Ability System Component when targeting an enemy.
		   
		   Breaking down this line:
		   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it
		      if not yet cached). We've already verified this is non-null in the if statement above.
		   
		   2. ->AbilityInputTagReleased(InputTag): Calls the custom method on our UFoxAbilitySystemComponent that
		      handles released input events. This method iterates through all activated abilities and calls
		      their input released callbacks, allowing abilities to respond to released input.
		   
		   3. InputTag parameter: The FGameplayTag for the left mouse button (InputTag_LMB). When targeting
		      is active (cursor is over a valid enemy), holding LMB should trigger targeting-related abilities
		      such as continuous attacks, channeled spells, or other combat actions directed at the enemy.
		   
		   Context: This code path is only reached when:
		     - InputTag matches InputTag_LMB (checked at the start of the function)
		     - bTargeting is true (meaning ThisActor is a valid enemy interface, set in AbilityInputTagPressed)
		     - The player has released the left mouse button
		   
		   This allows abilities bound to LMB to differentiate between targeting behavior (this path) and
		   auto-running behavior (when bTargeting is false). Once auto-run is removed, this special handling
		   may be simplified or removed entirely.
		*/
		GetASC()->AbilityInputTagReleased(InputTag);
	}
	
	// Checks if bTargeting (an enemy is being targeted) and bShiftKeyDown (the shift key is being held) are false, and
	// since the above if statement is false, the InputTag input parameter does match the left mouse button gameplay tag.
	// We Check for a short press to see if we should stop moving
	if (!bTargeting && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				
				if (NavPath->PathPoints.Num() > 0)
				{
					// This was commented out before and we got no error. Even uncommenting it does not cause an error
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				
					bAutoRunning = true;
				}
			}
			
			/*
			   Checks if the Ability System Component exists and if click visual effects are not blocked by gameplay tags.

			   Breaking down this line:
			   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it if not yet cached).
			      Returns nullptr if the controlled pawn doesn't have an ASC. This null check is necessary to prevent crashes
			      when attempting to call methods on a non-existent ASC.

			   2. &&: Logical AND operator with short-circuit evaluation. If GetASC() returns nullptr (false), the right side
			      of the expression is never evaluated, preventing a null pointer dereference when calling HasMatchingGameplayTag.

			   3. !GetASC()->HasMatchingGameplayTag(): The negation operator (!) inverts the boolean result from HasMatchingGameplayTag.
			      This means we only proceed if the blocking tag is NOT present on the ASC, allowing the visual feedback effect
			      to spawn when input is not blocked.

			   4. FFoxGameplayTags::Get().Player_Block_InputPressed: Retrieves the native C++ gameplay tag that indicates input
			      pressed events should be blocked. When this tag is NOT present on the ASC, it means the player is allowed to
			      see visual feedback for their click actions. This provides a centralized way to control whether click visual
			      effects should appear based on game state (e.g., during cutscenes, UI interactions, or crowd control effects,
			      we might want to suppress these visual indicators).

			   If both conditions are true (ASC exists AND the blocking tag is NOT present), the code proceeds to spawn the
			   click particle effect, providing visual confirmation of the player's click location. If either condition is
			   false, the visual effect is suppressed.
			 */
			if (GetASC() && !GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputPressed))
			{
				/*
				   Spawns a Niagara particle system at the clicked destination for visual feedback.
	
				   Breaking down this line:
				   1. UNiagaraFunctionLibrary::SpawnSystemAtLocation(): Static utility function that instantiates and
					  plays a Niagara particle system at a specified world location. This is a fire-and-forget effect
					  that plays once and automatically cleans itself up when the particle system completes.
	
				   2. this (first parameter - WorldContextObject): The world context object used to determine which
					  UWorld to spawn the particle system in. Passing 'this' (the player controller) provides the
					  necessary world context for the spawning operation.
	
				   3. ClickNiagaraSystem (second parameter - SystemTemplate): The UNiagaraSystem asset to spawn,
					  configured in the player controller blueprint. This defines the visual appearance of the click
					  feedback effect (e.g., a ground ripple, sparkle, or other indicator showing where the player
					  clicked).
	
				   4. CachedDestination (third parameter - Location): The FVector world position where the particle
					  system should be spawned. This is the final destination point calculated from the navigation
					  path, providing visual confirmation to the player of where their character will run to.
	
				   This provides immediate visual feedback for the auto-run destination when the player performs a
				   short click on the ground, helping players understand where their character will move before the
				   movement actually begins.
				 */
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);	
			}
		}
		FollowTime = 0.f;
		bTargeting = false;
	}
}

void AFoxPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	
	/*
	   Checks if input held events should be blocked based on gameplay tags applied to the Ability System Component.

	   Breaking down this line:
	   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it if not yet cached).
	      Returns nullptr if the controlled pawn doesn't have an ASC. This null check is necessary to prevent crashes
	      when attempting to call methods on a non-existent ASC.

	   2. &&: Logical AND operator with short-circuit evaluation. If GetASC() returns nullptr (false), the right side
	      of the expression is never evaluated, preventing a null pointer dereference when calling HasMatchingGameplayTag.

	   3. GetASC()->HasMatchingGameplayTag(): Method from UAbilitySystemComponent that checks if any gameplay tags
	      currently applied to this ASC match the provided tag. Returns true if a matching tag is found, false otherwise.
	      Gameplay tags on the ASC can be added by active gameplay effects, active abilities, or directly through code,
	      allowing dynamic control over which systems can process input.

	   4. FFoxGameplayTags::Get().Player_Block_InputHeld: Retrieves the native C++ gameplay tag that indicates input
	      held events should be blocked. When this tag is present on the ASC (e.g., applied by an ability like GA_Electrocute,
	      a crowd control effect, a cutscene, or UI state), it prevents the player from triggering continuous input-related 
	      actions. This provides a centralized way to disable held input processing without modifying every input handling function.

	   If both conditions are true (ASC exists AND the blocking tag is present), the function returns early without
	   forwarding the input to the ability system, effectively ignoring the player's held input.
	 */
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputHeld))
	{
		// Return early without processing the input held event, preventing ability held callbacks while input is blocked
		return;
	}
	
	// Checks if the InputTag input parameter is not the native C++ gameplay tag defined for the left mouse
	// button. We do this because we only want code after this if block to run if the input tag is for the LMB
	if (!InputTag.MatchesTagExact(FFoxGameplayTags::Get().InputTag_LMB))
	{
		// Checks if the ASC is not a null pointer
		if (GetASC())
		{
			/*
			   Forwards the held input event to the Ability System Component for processing.
			   
			   Breaking down this line:
			   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it
			      if not yet cached). We've already verified this is non-null in the if statement above.
			   
			   2. ->AbilityInputTagHeld(InputTag): Calls the custom method on our UFoxAbilitySystemComponent that
			      handles continuous input events. This method iterates through all activated abilities and calls
			      their input held callbacks, allowing abilities to respond to sustained input (e.g., charging
			      an attack, continuous channeling, or maintaining a blocking stance).
			   
			   3. InputTag parameter: The FGameplayTag identifying which input is being held (e.g., an ability
			      hotkey like InputTag_1, InputTag_2, etc.). The ASC uses this tag to determine which abilities
			      should respond to this held input event.
			   
			   Note: Left mouse button (LMB) input is intentionally excluded from this forwarding (checked above)
			   because LMB has special handling for targeting and auto-running behavior in this controller. This will be
			   removed.
			 */
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		// We just tried to activate the ability (or the ASC was null pointer) so we can return early
		return;
	}
	
	// Checks if bTargeting (an enemy is being targeted) or bShiftKeyDown (the shift key is being held) are true, and
	// since the above if statement is false, the InputTag input parameter does match the left mouse button gameplay tag.
	// We do this because we do not want the auto running behavior that follows this if block in this case
	if (bTargeting || bShiftKeyDown)
	{
		// Checks if the ASC is not a null pointer
		if (GetASC())
		{
			/*
			   Forwards the left mouse button held input to the Ability System Component when targeting an enemy.
			   
			   Breaking down this line:
			   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it
			      if not yet cached). We've already verified this is non-null in the if statement above.
			   
			   2. ->AbilityInputTagHeld(InputTag): Calls the custom method on our UFoxAbilitySystemComponent that
			      handles continuous input events. This method iterates through all activated abilities and calls
			      their input held callbacks, allowing abilities to respond to sustained input.
			   
			   3. InputTag parameter: The FGameplayTag for the left mouse button (InputTag_LMB). When targeting
			      is active (cursor is over a valid enemy), holding LMB should trigger targeting-related abilities
			      such as continuous attacks, channeled spells, or other combat actions directed at the enemy.
			   
			   Context: This code path is only reached when:
			     - InputTag matches InputTag_LMB (checked at the start of the function)
			     - bTargeting is true (meaning ThisActor is a valid enemy interface, set in AbilityInputTagPressed)
			     - The player is holding down the left mouse button
			   
			   This allows abilities bound to LMB to differentiate between targeting behavior (this path) and
			   auto-running behavior (when bTargeting is false). Once auto-run is removed, this special handling
			   may be simplified or removed entirely.
			*/
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	// Checks if bTargeting is false and if so implements the auto running behavior. Remove!!!
	else
	{
		// Increments follow time
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		// We do not care about this it will be removed
		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}
		
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UFoxAbilitySystemComponent* AFoxPlayerController::GetASC()
{
	if (FoxAbilitySystemComponent == nullptr)
	{
		/*
		   Retrieves and caches the Ability System Component (ASC) from the controlled pawn.
		   
		   Breaking down this line:
		   1. GetPawn<APawn>(): Gets the pawn currently controlled by this player controller and casts it to APawn type.
		      This is the player's character in the game world.
		   
		   2. UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(): A static utility function from Unreal's
		      Gameplay Ability System that retrieves the IAbilitySystemInterface from the provided actor and returns
		      its UAbilitySystemComponent. This works because our pawn implements the IAbilitySystemInterface.
		   
		   3. Cast<UFoxAbilitySystemComponent>(): Casts the returned UAbilitySystemComponent to our custom
		      UFoxAbilitySystemComponent subclass, giving us access to Fox-specific functionality like
		      AddCharacterAbilities() and custom input handling methods.
		   
		   This cached value prevents the need to repeatedly traverse the interface and perform casts every time
		   we need to access the ASC, improving performance when frequently accessing ability system functionality.
		 */
		FoxAbilitySystemComponent = Cast<UFoxAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	
	return FoxAbilitySystemComponent;
}

void AFoxPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(FoxContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer:: GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(FoxContext, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AFoxPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Casts the member variable InputComponent to type UFoxInputComponent
	UFoxInputComponent* FoxInputComponent = CastChecked<UFoxInputComponent>(InputComponent);
	
	/*
	   Binds the MoveAction input action to the Move function in this controller.
	   ETriggerEvent::Triggered means the Move function will be called continuously while the input is active.
	   This allows for smooth continuous movement as long as WASD keys are held down.
	 */
	FoxInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFoxPlayerController::Move);
	
	// Binds callback functions for ShiftAction input action that set the value of bShiftKeyPressed
	FoxInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AFoxPlayerController::ShiftPressed);
	FoxInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AFoxPlayerController::ShiftReleased);
	
	/*
	   Binds all ability input actions defined in the InputConfig data asset to their respective callback functions.
	   The InputConfig contains an array of FFoxInputAction structs, each mapping a UInputAction to a FGameplayTag.
	   This single call automatically binds each input action to three callbacks:
	     - AbilityInputTagPressed: Called once when the input is first pressed (ETriggerEvent::Started)
	     - AbilityInputTagReleased: Called once when the input is released (ETriggerEvent::Completed)
	     - AbilityInputTagHeld: Called continuously while the input is held down (ETriggerEvent::Triggered)
	   Each callback receives the FGameplayTag associated with the input action, allowing the ability system
	   to identify which ability should be activated, deactivated, or updated.
	 */
	FoxInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AFoxPlayerController::Move(const FInputActionValue& InputActionValue)
{
	/*
	   Checks if movement input should be blocked based on gameplay tags applied to the Ability System Component.

	   Breaking down this line:
	   1. GetASC(): Retrieves the cached UFoxAbilitySystemComponent pointer (or retrieves and caches it if not yet cached).
	      Returns nullptr if the controlled pawn doesn't have an ASC. This null check is necessary to prevent crashes
	      when attempting to call methods on a non-existent ASC.

	   2. &&: Logical AND operator with short-circuit evaluation. If GetASC() returns nullptr (false), the right side
	      of the expression is never evaluated, preventing a null pointer dereference when calling HasMatchingGameplayTag.

	   3. GetASC()->HasMatchingGameplayTag(): Method from UAbilitySystemComponent that checks if any gameplay tags
	      currently applied to this ASC match the provided tag. Returns true if a matching tag is found, false otherwise.
	      Gameplay tags on the ASC can be added by active gameplay effects, active abilities, or directly through code,
	      allowing dynamic control over which systems can process input.

	   4. FFoxGameplayTags::Get().Player_Block_InputPressed: Retrieves the native C++ gameplay tag that indicates input
	      pressed events should be blocked. When this tag is present on the ASC (e.g., applied by an ability like GA_Electrocute,
	      a crowd control effect, a cutscene, or UI state), we should prevent the player from moving their character. This provides
	      a centralized way to disable movement input without modifying the movement logic itself.

	   If both conditions are true (ASC exists AND the blocking tag is present), the function returns early without
	   processing movement, effectively immobilizing the character while preserving other input handling.
	 */
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputPressed))
	{
		// Return early without processing movement, effectively immobilizing the character while preserving other input handling
		return;
	}

	// Extracts the 2D input vector from WASD key presses, where X represents left/right input (A/D keys) and Y represents forward/backward input (W/S keys)
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	// Retrieves the current control rotation of the player controller, which represents the direction the camera is facing in the world
	const FRotator Rotation = GetControlRotation();
	
	// Zero pitch and roll but keep yaw
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	// X is forward in gameplay
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn())
	{
		// Y is forward in WASD input action
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}


