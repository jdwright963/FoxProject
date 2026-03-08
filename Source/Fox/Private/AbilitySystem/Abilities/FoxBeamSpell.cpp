// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxBeamSpell.h"

#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UFoxBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	// Check if the mouse cursor trace hit a blocking object in the world
	if (HitResult.bBlockingHit)
	{
		// Store the world location where the mouse cursor intersected with the blocking object
		MouseHitLocation = HitResult.ImpactPoint;
		
		// Store the actor that was hit by the mouse cursor
		MouseHitActor = HitResult.GetActor();
	}
	// Check if the mouse cursor trace did NOT hit a blocking object in the world
	else
	{
		// If no blocking hit occurred, cancel the ability as we need a valid target location
		// CurrentSpecHandle: The handle to the specific instance of this ability being executed
		// CurrentActorInfo: Contains information about the actor/avatar executing this ability, including:
		//   - OwnerActor: The actor that owns the AbilitySystemComponent (e.g., PlayerState)
		//   - AvatarActor: The physical actor in the world (e.g., Character)
		//   - PlayerController: The controller for player-controlled avatars
		//   - AbilitySystemComponent: The ASC that granted this ability
		// CurrentActivationInfo: Contains information about how this ability was activated, including:
		//   - ActivationMode: How the ability was triggered:
		//     * Predicting: Client predicted the ability activation before server confirmation (client-side prediction)
		//     * Authority: Server authoritatively activated the ability
		//     * Confirmed: Server confirmed a client's predicted activation (the prediction was accepted and is now authoritative)
		//   - bCanBeEndedByOtherInstance: Whether a different activation of this same ability can forcibly terminate this 
		//     instance (e.g., if the ability is activated again while already running)
		//   - PredictionKeyWhenActivated: The prediction key used for client-server synchronization
		// true: bReplicateCancelAbility - Whether to replicate the cancellation to clients in multiplayer
		// The first three input arguements are variables that are inherited from GameplayAbility.h
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UFoxBeamSpell::StoreOwnerVariables()
{
	// Check if CurrentActorInfo is valid before accessing its members
	// CurrentActorInfo is inherited from GameplayAbility.h and contains information about the actor executing this ability, including:
	//   - OwnerActor: The actor that owns the AbilitySystemComponent (e.g., PlayerState)
	//   - AvatarActor: The physical actor in the world (e.g., Character)
	//   - PlayerController: The controller for player-controlled avatars
	//   - AbilitySystemComponent: The ASC that granted this ability
	if (CurrentActorInfo)
	{
		// Store the PlayerController from CurrentActorInfo for later use in the ability
		// We use .Get() to safely retrieve the raw pointer from the weak pointer
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		
		// Store the AvatarActor (the physical actor in the world) as a Character reference
		// We convert from AActor to ACharacter because:
		//   - AvatarActor is a generic AActor pointer, which only provides basic actor functionality (transform, components, etc.)
		//   - ACharacter provides access to character-specific functionality needed for the beam spell:
		//     * Movement component for controlling character locomotion during ability execution
		//     * Mesh component for attaching visual effects to specific sockets (e.g., hand or weapon)
		//     * Animation blueprint integration for playing ability-specific animations
		//     * Capsule component for collision and positioning calculations relative to the character's bounds
		// We use Cast to safely convert the generic AActor pointer to ACharacter type
		// If the AvatarActor is not a Character (e.g., it's a different actor type), this will return nullptr
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UFoxBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	// Verify that OwnerCharacter is valid and was successfully cached in StoreOwnerVariables()
	// check() is a runtime assertion that will crash in development builds if the condition is false
	// This ensures we don't proceed with the trace if the character reference is null
	check(OwnerCharacter);
	
	// Check if the character implements the UCombatInterface, which provides access to combat related functionality.
	// The combat interface is required to retrieve the weapon mesh component for determining the beam's starting point
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		// Retrieve the weapon skeletal mesh component via the combat interface's GetWeapon function
		// The weapon mesh is needed to access the "TipSocket" which defines where the beam originates from (e.g., staff tip, wand tip)
		// Execute_ prefix is used to call BlueprintNativeEvent functions in C++
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			// Initialize an array to store actors that should be ignored during the sphere trace
			// This prevents the trace from hitting actors we don't want to target (like the caster themselves)
			TArray<AActor*> ActorsToIgnore;
			
			// Add the owner character to the ignore list so the beam trace doesn't hit the caster
			// Without this, the sphere trace would immediately collide with the character casting the beam
			ActorsToIgnore.Add(OwnerCharacter);
			
			// Declare a hit result structure to store information about what the sphere trace collides with
			// This will contain impact point, hit actor, surface normal, and other collision data if a hit occurs
			FHitResult HitResult;

			// Get the world space location of the "TipSocket" on the weapon mesh
			// This socket defines where the beam originates from (e.g., the tip of a staff or wand)
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));

			// Perform a sphere trace from the weapon tip to the beam target location
			// Parameters:
			//   - OwnerCharacter: World context object for the trace (specifies which world to perform the trace in)
			//   - SocketLocation: Start point of the trace (weapon tip)
			//   - BeamTargetLocation: End point of the trace (where the beam should go)
			//   - 10.f: Radius of the sphere in centimeters (creates a 20 unit diameter sphere for more forgiving hit detection)
			
			//   - TraceTypeQuery1: The trace channel to use for collision detection. This is an ETraceTypeQuery enum value that maps to
			//     a specific collision channel configured in Project Settings > Collision. TraceTypeQuery1 typically corresponds to the
			//     "Visibility" trace channel by default, which is commonly used for line-of-sight checks and projectile/beam targeting.
			//     Trace channels determine which objects the trace can detect based on their collision response settings:
			//       * Objects set to "Block" this channel will stop the trace and generate a blocking hit
			//       * Objects set to "Overlap" this channel will be detected but won't stop the trace
			//       * Objects set to "Ignore" this channel will be completely ignored by the trace
			//     The numeric suffix (1, 2, 3, etc.) corresponds to custom trace channels you can configure in your project.
			//     You can view and modify trace channel mappings in Project Settings > Engine > Collision > Trace Channels.
			//     TraceTypeQuery1 maps to Visibility (ECC_Visibility)
            //     TraceTypeQuery2 maps to Camera (ECC_Camera)
            //     TraceTypeQuery3 and above map to the Custom Channels you create in that list.
			
			//   - false: bTraceComplex - uses simple collision instead of complex (per-polygon) collision for better performance
			//   - ActorsToIgnore: Array of actors to skip during the trace (contains the caster to prevent self-hits)
			//   - EDrawDebugTrace::None: No debug visualization drawn (change to ForDuration or ForOneFrame for debugging)
			//   - HitResult: Output parameter that will be filled with collision information if a hit occurs
			//   - true: bIgnoreSelf - additional safety flag to ignore the trace instigator
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult,
				true);
			
			// Check if the sphere trace successfully hit a blocking object
			// bBlockingHit will be true when the trace collided with an object set to "Block" on the trace channel
			// If true, we store the hit information for targeting. If false, no valid target was found
			if (HitResult.bBlockingHit)
			{
				// Store the exact world location where the sphere trace impacted the target
				// This position will be used as the endpoint for the beam visual effect and damage calculation
				// We update MouseHitLocation here (after initially setting it in StoreMouseDataInfo()) because the sphere trace
				// from the weapon tip socket may hit a different point than the mouse cursor trace, providing a more accurate
				// impact location that accounts for the beam's actual trajectory from the character's weapon to the target.
				MouseHitLocation = HitResult.ImpactPoint;

				// Store the actor that was hit by the sphere trace as the beam's target
				// This actor reference will be used to apply damage, visual effects, and other gameplay consequences
				// We update MouseHitActor here (after initially setting it in StoreMouseDataInfo()) because the sphere trace
				// from the weapon tip socket may hit a different actor than the mouse cursor trace, ensuring we target the
				// actor that would actually be intersected by the beam's path rather than what the mouse cursor initially hit
				// For Example, an enemy could be standing between the character executing this ability and the enemy
				// they originally targeted with the mouse cursor and we do not want this ability to go through the enemy
				// inbetween (ignoring them) and hitting the initial target
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}

void UFoxBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	// Initialize an array to store actors that should be excluded when searching for additional beam targets
	// This prevents the beam from targeting actors that shouldn't be considered (like the caster or the primary target)
	TArray<AActor*> ActorsToIgnore;

	// Add the character executing this ability to the ignore list to prevent them from being selected as an additional beam target
	// GetAvatarActorFromActorInfo() retrieves the physical actor in the world from CurrentActorInfo
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());

	// Add the primary target (the actor hit by the initial beam trace) to the ignore list
	// This ensures the already-targeted actor isn't included in the additional targets list, preventing duplicate targeting
	ActorsToIgnore.Add(MouseHitActor);
	
	// Declare an array to store all living actors found within the search radius
	// This will be populated by GetLivePlayersWithinRadius() and used as the candidate pool for additional beam targets
	TArray<AActor*> OverlappingActors;

	// Find all living actors within a 850 unit radius of the primary beam target's location
	// Parameters:
	//   - GetAvatarActorFromActorInfo(): The world context object (the actor executing this ability) this tells this
	//     static function in which world to execute the search
	//   - OverlappingActors: Output array that will be filled with all valid actors found in the radius
	//   - ActorsToIgnore: Array of actors to exclude from the search (caster and primary target)
	//   - 850.f: Search radius in Unreal units, which defines how far from the primary target we look for additional targets
	//   - MouseHitActor->GetActorLocation(): Center point of the search sphere (the primary target's position)
	UFoxAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation());

	// Calculate the maximum number of additional targets the beam can chain to based on ability level
	// Uses FMath::Min to cap the value at MaxNumShockTargets (5) to prevent excessive chaining at high levels
	// Formula: (AbilityLevel - 1) means at level 1 we get 0 additional targets, level 2 gets 1, level 3 gets 2, etc.
	// This creates natural ability scaling where higher level abilities can hit more targets simultaneously
	// Example: Level 3 ability = Min(3-1, 5) = 2 additional targets; Level 7 ability = Min(7-1, 5) = 5 additional targets
	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);

	// Select the closest NumAdditionalTargets actors from the OverlappingActors pool and store them in OutAdditionalTargets
	// Parameters:
	//   - NumAdditionalTargets: Maximum number of additional targets to select (calculated above based on ability level)
	//   - OverlappingActors: The pool of candidate actors found within the search radius
	//   - OutAdditionalTargets: Output array that will contain the final list of additional beam targets, sorted by proximity
	//   - MouseHitActor->GetActorLocation(): Reference point for distance calculations (primary target's position)
	// If there are fewer actors in OverlappingActors than NumAdditionalTargets, all available actors will be selected
	UFoxAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());
}
