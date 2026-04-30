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

FDamageEffectParams UFoxDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride,
	bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride, bool bOverridePitch, float PitchOverride) const
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
		
		// Check if the caller has requested to override the calculated pitch angle with a custom value
		// This allows precise control over the vertical launch angle independently of the target's position
		// Useful for abilities that need consistent upward/downward trajectory regardless of target height
		if (bOverridePitch)
		{
			// Replace the calculated pitch component with the provided override value
			// This directly sets the vertical angle of the force/impulse direction to the desired custom pitch
			// For example, setting PitchOverride to 45.f will launch targets at a 45-degree upward angle
			Rotation.Pitch = PitchOverride;
		}

		// Convert the modified rotation back into a normalized direction vector
		// .Vector() transforms the rotator into a unit vector (magnitude of 1.0) pointing in that direction
		// This normalized vector represents the direction we want to apply force, before scaling by magnitude
		const FVector ToTarget = Rotation.Vector();
		
		// Check if we should use the default calculated direction (from source to target) for knockback force
		// If NOT overriding, we use the natural direction based on the target's position relative to the ability source
		// This creates intuitive physics where targets are knocked away from the source of the damage
		if (!bOverrideKnockbackDirection)
		{
			// Calculate the knockback force vector by scaling the normalized direction with the configured magnitude
			// ToTarget provides the direction (where to push), KnockbackForceMagnitude provides the strength (how hard to push)
			// This results in a force that pushes the target away from the ability source at the specified strength
			Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
		}
		// Check if we should use the default calculated direction (from source to target) for death impulse
		// If NOT overriding, the ragdoll will be launched in the natural direction away from the damage source
		// This creates a realistic death animation where corpses fly backward from the impact point
		if (!bOverrideDeathImpulse)
		{
			// Calculate the death impulse vector by scaling the normalized direction with the configured magnitude
			// ToTarget provides the launch direction, DeathImpulseMagnitude provides the launch strength
			// This impulse is applied to the physics-simulated mesh upon death, creating the ragdoll launch effect
			Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
	}
	
	// Check if the caller wants to use a custom direction for knockback instead of the default source-to-target direction
	// This allows abilities to knock targets in arbitrary directions (e.g., always upward, toward a specific point, or along a surface normal)
	// Useful for abilities like explosions that push targets away from a central point, or jump pads that launch in a fixed direction
	if (bOverrideKnockbackDirection)
	{
		// Normalize the override direction vector to ensure it has a magnitude of exactly 1.0 (unit vector)
		// This is essential because we multiply by KnockbackForceMagnitude afterward - the direction must be unit length
		// Without normalization, a vector like (10, 0, 0) would result in 10x stronger knockback than intended
		// Normalize() modifies the vector in-place, converting it to the same direction but with length 1.0
		KnockbackDirectionOverride.Normalize();

		// Calculate the knockback force by scaling the normalized custom direction with the configured magnitude
		// This gives us a force vector pointing in the specified override direction at the desired strength
		// For example, if override is (0, 0, 1) and magnitude is 1000, this creates an upward force of 1000 units
		Params.KnockbackForce = KnockbackDirectionOverride * KnockbackForceMagnitude;

		// Check if we also need to apply a custom pitch angle to the knockback direction override
		// This allows fine-tuning the vertical component of the custom direction independently
		// Even when using a custom horizontal direction, we might want to adjust the launch angle for gameplay feel
		if (bOverridePitch)
		{
			// Convert the normalized override direction vector into a rotator representation
			// This transformation gives us access to the pitch, yaw, and roll components of the direction
			// We need rotator form to manipulate the pitch angle component separately from the overall direction
			FRotator KnockbackRotation = KnockbackDirectionOverride.Rotation();

			// Replace the calculated pitch component with the custom pitch override value
			// This directly sets the vertical launch angle to the desired value (e.g., 45 degrees for a diagonal launch)
			// Allows precise control over trajectory arc independently of the base knockback direction
			KnockbackRotation.Pitch = PitchOverride;

			// Convert the pitch-modified rotation back into a normalized direction vector and scale by magnitude
			// .Vector() transforms the rotator into a unit direction vector with the new pitch applied
			// We then multiply by the magnitude to get the final force vector with both custom direction and custom pitch
			// This overwrites the earlier knockback force calculation with the pitch-adjusted version
			Params.KnockbackForce = KnockbackRotation.Vector() * KnockbackForceMagnitude;
		}
	}

	// Check if the caller wants to use a custom direction for death impulse instead of the default source-to-target direction
	// This allows ragdolls to be launched in specific directions regardless of where the damage came from
	// Useful for abilities with cinematic death effects (e.g., explosive barrels always launch upward, trap spikes launch forward)
	if (bOverrideDeathImpulse)
	{
		// Normalize the death impulse override direction vector to ensure it's a unit vector (magnitude of 1.0)
		// This is critical for consistent physics behavior - the direction must be normalized before scaling by magnitude
		// Without normalization, the actual impulse strength would vary based on the input vector's length
		// Normalize() modifies the vector in-place to maintain direction while setting length to exactly 1.0
		DeathImpulseDirectionOverride.Normalize();

		// Calculate the death impulse by scaling the normalized custom direction with the configured magnitude
		// This produces an impulse vector pointing in the specified override direction with the desired launch strength
		// For example, an upward override (0, 0, 1) with magnitude 10000 launches ragdolls straight up with strong force
		Params.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude;

		// Check if we also need to apply a custom pitch angle to the death impulse direction override
		// This enables separate control of the vertical launch angle even when using a custom base direction
		// Allows designers to fine-tune ragdoll trajectory arc for optimal visual impact
		if (bOverridePitch)
		{
			// Convert the normalized override direction vector into rotator form (pitch, yaw, roll components)
			// This transformation allows us to access and modify the pitch angle independently
			// Rotator representation is necessary to manipulate individual rotation components
			FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation();

			// Override the calculated pitch component with the custom pitch value
			// This sets the exact vertical launch angle (e.g., 30 degrees for a shallow arc, 60 for steep)
			// Provides precise control over how high vs. how far the ragdoll travels
			DeathImpulseRotation.Pitch = PitchOverride;

			// Convert the pitch-modified rotation back to a normalized direction vector and scale by magnitude
			// .Vector() converts the rotator with the new pitch into a unit direction vector
			// Multiplying by magnitude gives the final impulse vector with both custom direction and custom pitch applied
			// This replaces the earlier death impulse calculation with the pitch-adjusted version
			Params.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	
	// Check if this ability is configured to deal radial/area-of-effect damage instead of single-target damage
	// bIsRadialDamage is a UPROPERTY boolean set in the ability's Blueprint or class defaults
	// When true, damage is applied to all actors within a spherical radius rather than just one target
	// This enables abilities like explosions, shockwaves, or area denials that affect multiple targets simultaneously
	if (bIsRadialDamage)
	{
		// In the params struct enable radial damage mode by copying the class default boolean value
		// This flag tells the damage application system to use radial damage calculations instead of direct application
		// The receiving system will iterate through all actors within the specified radius and apply damage based on distance
		Params.bIsRadialDamage = bIsRadialDamage;

		// In the params struct set the world-space origin point from which radial damage emanates and distance calculations are measured
		// RadialDamageOrigin is typically set to a projectile impact location, spell effect center, or explosion epicenter
		// All targets' distances are calculated from this point to determine damage falloff. The value it is set to is 
		// determined by the input parameter InRadialDamageOrigin.
		Params.RadialDamageOrigin = InRadialDamageOrigin;
		
		// In the params struct set the inner radius (in Unreal units) within which targets receive 100% of the base damage with no falloff
		// Actors inside this radius are in the "full damage zone" and take maximum damage regardless of their exact position
		// This creates a guaranteed lethal/high-damage core area for powerful abilities like explosions
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;

		// In the params struct set the outer radius (in Unreal units) that defines the maximum range of the radial damage effect
		// Actors between the inner and outer radius receive damage that linearly interpolates from 100% to 0% based on distance
		// Actors beyond this radius take no damage at all, making this the ability's effective area-of-effect boundary
		// The falloff zone (between inner and outer radius) allows for more realistic and balanced damage distribution
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	
	// Return the fully populated damage parameters struct to the caller
	// The caller (typically a Blueprint or C++ function) can now pass this struct to damage application systems
	// without needing to manually gather and configure all these individual values
	// This encapsulation makes damage dealing consistent and reduces the chance of configuration errors
	return Params;
}

float UFoxDamageGameplayAbility::GetDamageAtLevel() const
{
	// Return the scaled damage value by evaluating the Damage FScalableFloat at the ability's current level
	// This retrieves the damage magnitude after applying any curve table calculations for level-based scaling
	// Used by external systems that need to query this ability's damage output without applying effects
	return Damage.GetValueAtLevel(GetAbilityLevel());
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
