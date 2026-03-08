// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UFoxDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Create a Gameplay Effect Spec and a Handle for it from the DamageEffectClass with ability level set to 1.0
	// This spec is a container for the gameplay effect that will be applied to the target
	// The level parameter (1.f) is used for any level-based calculations within the effect itself
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	
	// Calculate the final damage value by evaluating the Damage FScalableFloat at the current ability level
	// FScalableFloat is a powerful UE struct that combines a base float value with an optional UCurveTable reference
	// This allows designers to define damage progression curves in data tables without touching code
	// GetValueAtLevel() looks up the curve at the specified level (GetAbilityLevel() is a function this class inherits)
	// If no curve is assigned, it simply returns the base value. If a curve exists, it evaluates it at that level
	// This pattern enables flexible damage scaling: linear, exponential, or any custom curve shape
	// For example, a fireball might deal 10 damage at level 1, but 50 damage at level 10 via a curve
	const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());

	// Assign the calculated damage magnitude to the effect spec using the SetByCaller system with the DamageType tag as the key
	// The SetByCaller system is GAS's mechanism for passing dynamic runtime values into gameplay effects
	// Instead of hardcoding values in the effect class, we "set by caller" at the moment of application
	// AssignTagSetByCallerMagnitude() stores a <GameplayTag, float> pair in the effect spec's SetByCallerMagnitudes map
	// The DamageType tag (e.g., "Damage.Fire" or "Damage.Lightning") acts as the lookup key
	// When the DamageEffectClass is applied, its modifiers can use GetSetByCallerMagnitude(DamageType) to retrieve ScaledDamage
	// This pattern allows one generic damage effect class to handle all damage types by reading the tag-identified magnitude
	// It's more flexible and maintainable than creating separate UGameplayEffect subclasses for each damage type
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaledDamage);
	
	/*
	 * Apply the fully configured damage effect spec to the target actor's Ability System Component
	 * This complex line performs several operations in sequence:
	 *
	 * 1. GetAbilitySystemComponentFromActorInfo():
	 *    - This function is inherited from UGameplayAbility base class
	 *    - It retrieves the Ability System Component (ASC) of the actor who OWNS/ACTIVATED this ability
	 *    - The "ActorInfo" is a struct (FGameplayAbilityActorInfo) that stores cached references to key actors:
	 *      * OwnerActor: The actor that owns the ASC (e.g., the player character or AI)
	 *      * AvatarActor: The physical actor in the world (often same as owner, but can differ for possessed pawns)
	 *      * PlayerController: The controller if this is a player-controlled character
	 *      * AbilitySystemComponent: The ASC itself that grants and manages abilities
	 *    - This function returns a pointer to the instigator's/source's ASC
	 *
	 * 2. ->ApplyGameplayEffectSpecToTarget():
	 *    - This is a member function of UAbilitySystemComponent
	 *    - It takes two parameters: a FGameplayEffectSpec (by reference) and a target UAbilitySystemComponent pointer
	 *    - This function is responsible for applying a gameplay effect from one ASC (source) to another ASC (target)
	 *    - It handles all the gameplay effect application logic including:
	 *      * Checking if the effect can be applied (tags, immunity, etc.)
	 *      * Creating an active gameplay effect handle
	 *      * Executing the effect's modifiers on the target's attributes
	 *      * Managing duration, period, and stacks if applicable
	 *      * Broadcasting delegates to notify systems that an effect was applied
	 *
	 * 3. *DamageSpecHandle.Data.Get():
	 *    - DamageSpecHandle is of type FGameplayEffectSpecHandle (a wrapper/smart pointer for FGameplayEffectSpec)
	 *    - The handle pattern is used for safe memory management and replication
	 *    - .Data is a TSharedPtr<FGameplayEffectSpec> member variable inside FGameplayEffectSpecHandle
	 *    - TSharedPtr is Unreal's reference-counted smart pointer (similar to std::shared_ptr)
	 *    - .Get() is a TSharedPtr function that returns the raw pointer to the actual FGameplayEffectSpec object
	 *    - The asterisk (*) dereferences this raw pointer to get the actual FGameplayEffectSpec object
	 *    - ApplyGameplayEffectSpecToTarget expects a const FGameplayEffectSpec& (reference), not a pointer
	 *    - This dereferencing is necessary to convert from pointer to reference for the function parameter
	 *
	 * 4. UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor):
	 *    - UAbilitySystemBlueprintLibrary is a static Blueprint Function Library for GAS utilities
	 *    - GetAbilitySystemComponent() is a static helper function that safely retrieves an ASC from any actor
	 *    - It implements the IAbilitySystemInterface to find the ASC on the target actor
	 *    - The interface pattern allows different actor classes to store their ASC in different ways
	 *    - This function handles the polymorphism and returns nullptr if the actor doesn't have an ASC
	 *    - TargetActor is the AActor* parameter passed into CauseDamage() - the actor receiving the damage
	 *    - This returns the TARGET's ASC (as opposed to the source ASC from step 1)
	 *
	 * In summary: This line applies a damage effect FROM the ability owner's ASC TO the target actor's ASC
	 * The source ASC (ability owner) applies the configured damage spec to the target ASC (damage recipient)
	 */
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

FDamageEffectParams UFoxDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor) const
{
	// Declare and default constructs a FDamageEffectParams struct to aggregate all damage-related configuration
	// This struct serves as a data container that bundles together all the parameters needed for damage application
	// By using a struct instead of individual parameters, we achieve:
	// 1. Cleaner function signatures (one parameter instead of 10+ individual values)
	// 2. Easier maintenance (adding new damage parameters only requires updating the struct)
	// 3. Type safety (can't accidentally swap parameter order when passing values around)
	// 4. Self-documenting code (struct member names clearly indicate what each value represents)
	// The following code populates this struct with values from this ability's class defaults and runtime context
	// before returning it to the caller for use in damage application systems
	FDamageEffectParams Params;

	// Set the world context object to the avatar actor (the physical actor executing this ability in the world)
	// This is required for spawning gameplay cues (visual/audio effects) at the correct location
	// GetAvatarActorFromActorInfo() returns the "avatar" from the cached ability actor info. This is typically
	// the character/pawn that's performing the ability, as opposed to the controller or ability system component
	Params.WorldContextObject = GetAvatarActorFromActorInfo();

	// Assign the damage effect class from this ability's configuration to be used when applying damage
	// DamageEffectClass is a UPROPERTY set in Blueprint and defines which UGameplayEffect to instantiate
	// By using the class default, we ensure consistency. All damage applications from this ability use the same effect
	Params.DamageGameplayEffectClass = DamageEffectClass;

	// Store the source ASC (the ability owner's/instigator's Ability System Component) for effect application
	// This identifies WHO is dealing the damage and is used for gameplay effect context (source tags, attributes, etc.)
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();

	// Retrieve and store the target's ASC using the Blueprint library helper function
	// This will be nullptr if TargetActor is null or doesn't implement IAbilitySystemInterface
	// The calling code is responsible for validating this pointer before attempting to apply damage effects
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	// Calculate the base damage by evaluating the FScalableFloat at the current ability level
	// This uses the same damage scaling system as CauseDamage(). The curve table lookup enables level-based progression
	// The "Base" prefix indicates this is the raw damage before any modifications from resistances, buffs, or critical hits
	Params.BaseDamage = Damage.GetValueAtLevel(GetAbilityLevel());

	// Store the current ability level for use in effect calculations and damage formulas
	// Many gameplay effects need the ability level for scaling calculations beyond just the base damage
	// For example, debuff potency or critical hit chance might also scale with ability level
	Params.AbilityLevel = GetAbilityLevel();

	// Assign the damage type gameplay tag (e.g., Damage.Fire, Damage.Lightning) for the SetByCaller system
	// This tag serves as the lookup key when the damage effect reads GetSetByCallerMagnitude() to retrieve damage values
	// It also categorizes the damage for resistance calculations and debuff type determination
	Params.DamageType = DamageType;

	// Set the percentage chance (0-100) that this damage application will also apply the associated debuff effect
	// For example, a fire ability with 20.f DebuffChance has a 20% probability to apply the Burn debuff on hit
	// This is used with SetByCaller via the Debuff.Chance tag when the gameplay effect is applied
	Params.DebuffChance = DebuffChance;

	// Set the damage dealt per tick while the debuff is active on the target
	// This periodic damage value is applied at each DebuffFrequency interval throughout the DebuffDuration
	// Used with SetByCaller via the Debuff.Damage tag to configure the debuff's damage-over-time component
	Params.DebuffDamage = DebuffDamage;

	// Set the total time (in seconds) that the debuff persists on the target after successful application
	// Combined with DebuffFrequency, this determines the total number of damage ticks the debuff will inflict
	// Used with SetByCaller via the Debuff.Duration tag to configure the gameplay effect's duration
	Params.DebuffDuration = DebuffDuration;

	// Set the time interval (in seconds) between each debuff damage tick
	// A value of 1.f means damage is applied once per second. 0.5f would apply damage twice per second
	// Used with SetByCaller via the Debuff.Frequency tag to configure the gameplay effect's period
	Params.DebuffFrequency = DebuffFrequency;
	
	/**
	 * Set the Magnitude of the physics impulse applied to the target actor's mesh upon death.
	 * 
	 * When a character dies from this damage, this value determines the strength of the physical
	 * force applied to their physics-simulated mesh, creating a ragdoll "launch" effect. The impulse
	 * is typically applied in the direction from the damage source to the target, making enemies
	 * fly backward when killed.
	*/
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	
	// Set the magnitude scalar for knockback force calculations. This value determines how strong the knockback 
	// effect will be
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;

	// Set the percentage chance (0-100) that a successful hit will trigger the knockback effect
	Params.KnockbackChance = KnockbackChance;

	// Validate that the target actor pointer is not null and the actor hasn't been destroyed
	// This check is essential before accessing the target's location to prevent null pointer crashes
	// IsValid() checks both pointer validity and whether the UObject has been marked for garbage collection
	if (IsValid(TargetActor))
	{
		// Calculate the directional rotation from the ability source (avatar) to the target actor
		// Subtracting source location from target location gives us a vector pointing toward the target
		// .Rotation() converts this direction vector into a rotator (pitch, yaw, roll representation)
		// This rotation will be used to apply impulse/force in the direction the target is from the source
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();

		// Override the pitch (up and down) component to 45 degrees to create an upward launch trajectory
		// Without this, targets would be pushed purely horizontally (pitch = 0)
		// A 45-degree angle provides a balanced arc that launches targets both away and upward
		// This creates more dramatic and visually appealing death/knockback effects
		Rotation.Pitch = 45.f;

		// Convert the modified rotation back into a normalized direction vector
		// .Vector() transforms the rotator into a unit vector (magnitude of 1.0) pointing in that direction
		// This normalized vector represents the direction we want to apply force, before scaling by magnitude
		const FVector ToTarget = Rotation.Vector();

		// Calculate the final death impulse vector by scaling the direction vector by the death impulse magnitude
		// This creates the actual force vector that will be applied to the target's physics body on death
		// The result is a vector pointing 45 degrees upward toward the target with the specified magnitude
		Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;

		// Calculate the final knockback force vector by scaling the direction vector by the knockback magnitude
		// Similar to death impulse but used for non-lethal knockback effects during combat
		// This force is applied when the knockback chance succeeds, pushing targets away without killing them
		// The result is a vector pointing 45 degrees upward toward the target with the specified magnitude
		Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
	}

	// Return the fully populated damage parameters struct to the caller
	// The caller (typically a Blueprint or C++ function) can now pass this struct to damage application systems
	// without needing to manually gather and configure all these individual values
	// This encapsulation makes damage dealing consistent and reduces the chance of configuration errors
	return Params;
}

FTaggedMontage UFoxDamageGameplayAbility::GetRandomTaggedMontageFromArray(
	const TArray<FTaggedMontage>& TaggedMontages) const
{
	// Check if the TaggedMontages array contains any elements before attempting to select one
	// This validation is critical to prevent accessing an invalid array index
	if (TaggedMontages.Num() > 0)
	{
		// Generate a random integer index within the valid range of the array
		// FMath::RandRange(Min, Max) returns a random integer that is inclusive of both Min and Max bounds
		// We use 0 as the minimum (first array index) and TaggedMontages.Num() - 1 as the maximum (last valid index)
		// For example, if the array has 5 elements (indices 0-4), this will randomly select an index between 0 and 4
		// The subtraction of 1 is necessary because array indices are zero-based while Num() returns the count
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);

		// Return the FTaggedMontage element at the randomly selected index
		// This provides a random selection from all available tagged montages in the array
		// The array access operator [] is safe here because we've validated the array is not empty
		// and our Selection index is guaranteed to be within valid bounds [0, Num()-1]
		return TaggedMontages[Selection];
	}
	// If the array is empty (contains no elements), return a default-constructed FTaggedMontage object
	// FTaggedMontage() calls the default constructor which initializes all member variables to their default values
	// This acts as a safe fallback that prevents crashes or undefined behavior when no montages are available
	// The calling code should check if the returned FTaggedMontage is valid before attempting to use it
	// (e.g., checking if the montage pointer is not null or if the associated gameplay tag is valid)
	return FTaggedMontage();
}
