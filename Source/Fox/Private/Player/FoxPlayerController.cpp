// Copyright TryingToMakeGames


#include "Player/FoxPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "FoxGameplayTags.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Fox/Fox.h"
#include "GameFramework/Character.h"
#include "Input/FoxInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
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
	
	// Updates the magic circle actor's world position to match the cursor's impact point
	UpdateMagicCircleLocation();
}

void AFoxPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	// Checks if the MagicCircle actor has not been spawned yet or has been destroyed since the last call to ShowMagicCircle()
	if (!IsValid(MagicCircle))
	{
		// Spawns a new instance of the AMagicCircle actor class in the world using the MagicCircleClass template set in the blueprint
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		
		/*
		   Checks if a custom decal material was provided as a parameter to this function ShowMagicCircle().

		   The DecalMaterial parameter is optional (defaults to nullptr as specified in the function signature).
		   If a valid material is provided, it will override the default material set on the MagicCircleDecal
		   component in the AMagicCircle blueprint. This allows different abilities to use different visual
		   styles for their targeting circles (e.g., fire abilities might use a red circle, ice abilities
		   might use a blue circle) without requiring separate magic circle actor classes for each variation.

		   If DecalMaterial is nullptr, the magic circle will use whatever default material was configured
		   in its blueprint, maintaining consistent visual feedback when no custom material is specified.
		 */
		if (DecalMaterial)
		{
			/*
			   Applies the custom decal material to the magic circle's decal component.

			   Breaking down this line:
			   1. MagicCircle->MagicCircleDecal: Accesses the UDecalComponent member variable of the spawned
			      AMagicCircle actor. This component is responsible for projecting the magic circle decal onto
			      surfaces in the world, providing visual feedback for ability targeting.

			   2. ->SetMaterial(): Method from UDecalComponent that changes the material displayed by the decal.
			      This updates the visual appearance of the targeting circle.

			   3. 0 (first parameter - ElementIndex): The material slot index to modify. Decal components can have
			      multiple material slots, but we only use slot 0 since the magic circle only needs a single
			      material to define its appearance.

			   4. DecalMaterial (second parameter - Material): The UMaterialInterface pointer passed to this function
			      ShowMagicCircle() that defines the visual appearance of the targeting circle. This material
			      determines the color, pattern, opacity, and other visual properties of the decal projected
			      onto the ground or surfaces beneath the cursor.
			*/
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void AFoxPlayerController::HideMagicCircle()
{
	/*
	   Checks if the MagicCircle pointer is valid before attempting to destroy it.

	   IsValid() verifies that MagicCircle is not null and not pending kill (Destroy() has not already been called on it).
	   This prevents crashes from attempting to destroy a null pointer or an actor that is already being destroyed,
	   which could occur if HideMagicCircle() is called multiple times in succession or if the magic circle was
	   never spawned by ShowMagicCircle() in the first place.
	 */
	if (IsValid(MagicCircle))
	{
		/*
		   Destroys the magic circle actor and removes it from the world.

		   Destroy() marks the actor for deletion and removes it from the world at the end of the current frame.
		   This cleans up the visual representation of the magic circle when it's no longer needed (e.g., when
		   the player cancels a targeted ability or completes casting). The actor's memory will be garbage
		   collected by Unreal's object system after it's fully removed from the world.
		 */
		MagicCircle->Destroy();
	}
}

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

void AFoxPlayerController::UpdateMagicCircleLocation()
{
	/*
	   Checks if the MagicCircle pointer is valid before attempting to update its location.

	   IsValid() verifies that MagicCircle is not null and not pending kill (Destroy() has not been called on it).
	   This prevents crashes from accessing a destroyed or non-existent actor, which could occur if HideMagicCircle()
	   was called to destroy the magic circle, or if ShowMagicCircle() was never called to spawn it in the first place.
	 */
	if (IsValid(MagicCircle))
	{
		/*
		   Updates the magic circle's world position to follow the cursor's impact point on the ground.

		   CursorHit.ImpactPoint contains the FVector world location where the cursor trace hit the geometry,
		   calculated in the CursorTrace() function. By continuously updating (since we call this function in PlayerTick()) 
		   the magic circle's location to this point, the decal follows the player's cursor in real-time, providing 
		   visual feedback for targeted ability placement or ground-targeted spell casting.
		 */
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}

// We do not want the parts of this cursor trace that highlights actors in the final game this all needs to be removed
void AFoxPlayerController::CursorTrace()
{
	// Checks if the ASC exists and has the Player_Block_CursorTrace tag, which blocks cursor trace processing
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_CursorTrace))
	{
		// Removes highlight from the actor that was under the cursor in the previous frame
		UnHighlightActor(LastActor);
		
		// Removes highlight from the actor currently under the cursor
		UnHighlightActor(ThisActor);
		
		// Checks if ThisActor is valid and implements the HighlightInterface (incomplete condition statement)
		// if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())

		// Clears the LastActor reference to indicate no actor was under the cursor in the previous frame
		LastActor = nullptr;
		
		// Clears the ThisActor reference to indicate no actor is currently under the cursor
		ThisActor = nullptr;
		
		// Returns early to prevent cursor trace processing when it's blocked by gameplay tags
		return;
	}

	// Sets the trace channel to ECC_ExcludePlayers if MagicCircle is valid, otherwise uses ECC_Visibility
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;

	// Performs a line trace using TraceChannel from the cursor's screen position into the world and stores the hit result in CursorHit
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	
	// Returns early if the cursor trace didn't hit any valid blocking geometry in the world
	if (!CursorHit.IsValidBlockingHit()) return;
	
	/*
	   Stores the actor from the previous frame's cursor trace into LastActor for comparison.

	   This assignment preserves the actor that was under the cursor in the previous frame, allowing us to detect
	   when the cursor has moved from one actor to another (or from an actor to empty space, or vice versa). By
	   comparing LastActor to the newly retrieved ThisActor, we can determine whether highlighting state needs to
	   change, unhighlighting the previously hovered actor and/or highlighting the newly hovered actor.
	 */
	LastActor = ThisActor;
	
	// Checks if the actor hit by the cursor trace is valid and implements the HighlightInterface
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		// Stores the hit actor in ThisActor since it can be highlighted
		ThisActor = CursorHit.GetActor();
	}
	// The hit actor is either invalid or doesn't implement the HighlightInterface
	else
	{
		// Clears ThisActor since the hit actor cannot be highlighted
		ThisActor = nullptr;
	}
	
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
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

void AFoxPlayerController::HighlightActor(AActor* InActor)
{
	// Checks if InActor is valid and implements the HighlightInterface before attempting to highlight it
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		// Calls the HighlightActor implementation on the actor to apply the visual highlight effect
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AFoxPlayerController::UnHighlightActor(AActor* InActor)
{
	// Checks if InActor is valid and implements the HighlightInterface before attempting to unhighlight it
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		// Calls the UnHighlightActor implementation on the actor to remove the visual highlight effect
		IHighlightInterface::Execute_UnHighlightActor(InActor);
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
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
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
	
	// Checks if we're not targeting an enemy and shift key is not held, allowing auto-run behavior to trigger
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		// Retrieves the pawn currently controlled by this player controller
		const APawn* ControlledPawn = GetPawn();

		// Checks if this was a short press and the pawn exists
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// Checks if ThisActor is valid and implements the HighlightInterface, indicating it's an interactable object
			// like a checkpoint that can receive move-to location data
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				// Calls the SetMoveToLocation function on the actor implementing HighlightInterface, passing 
				// CachedDestination to set the target location for movement or interaction
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			// Checks if the ASC exists and the Player_Block_InputPressed tag is not present, allowing visual feedback for clicks when input is not blocked
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FFoxGameplayTags::Get().Player_Block_InputPressed))
			{
				// Spawns a Niagara particle system at the clicked location to provide visual feedback to the player indicating where they clicked on the ground
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			
			// Calculates a navigation path from the controlled pawn's current location to the clicked destination
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				// Removes all existing spline points to prepare for the new navigation path
				Spline->ClearSplinePoints();

				// Iterates through each point in the navigation path and adds it to the spline in world space coordinates
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				
				// Checks if the navigation path contains at least one point
				if (NavPath->PathPoints.Num() > 0)
				{
					// Caches the final destination point from the navigation path for auto-run distance checking
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];

					// Enables auto-run behavior so the character will follow the spline path
					bAutoRunning = true;
				}
			}
		}
		// Resets the follow time counter to zero now that the left mouse button has been released
		FollowTime = 0.f;

		// Clears the targeting status since the left mouse button has been released
		TargetingStatus = ETargetingStatus::NotTargeting;
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
	// Checks if we're targeting an enemy or shift key is held, enabling ability activation instead of auto-run
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
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


