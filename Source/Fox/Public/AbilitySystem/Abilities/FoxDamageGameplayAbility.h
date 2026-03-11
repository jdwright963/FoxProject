// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "FoxAbilityTypes.h"
#include "AbilitySystem/Abilities/FoxGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "FoxDamageGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxDamageGameplayAbility : public UFoxGameplayAbility
{
	GENERATED_BODY()
public:

	/**
	 * Applies damage to a target actor using the configured DamageEffectClass and DamageTypes.
	 * 
	 * UFUNCTION(BlueprintCallable) exposes this C++ function to Blueprints as a callable node.
	 * 
	 * When called in Blueprint, this function generates TWO input pins:
	 * 
	 * 1. "Target" pin (implicit, execution pin context):
	 *    - Type: UFoxDamageGameplayAbility reference
	 *    - Default value: Self
	 *    - This represents the OBJECT instance on which this function is being called
	 *    - Since this is a non-static member function, it requires an instance of UFoxDamageGameplayAbility to execute
	 *    - In Blueprint, this is the "self" reference - the ability instance that owns and executes this function
	 *    - This pin provides access to all member variables (DamageEffectClass, DamageTypes) and inherited 
	 *      ability context (ability level, owner's ASC, etc.)
	 * 
	 * 2. "Target Actor" pin (explicit parameter):
	 *    - Type: AActor* (Actor Object Reference)
	 *    - This is the explicit function parameter defined in the C++ signature
	 *    - Represents the actor that will RECEIVE the damage
	 *    - Must be connected to an actor reference in Blueprint (e.g., from a collision hit result, target selection, etc.)
	 *    - The function retrieves this actor's Ability System Component and applies the damage effect to it
	 * 
	 * The distinction is crucial:
	 * - "Target" (self) = The ability instance doing the damage (source/instigator)
	 * - "TargetActor" = The actor receiving the damage (recipient/victim)
	 * 
	 * @param TargetActor The actor to receive damage. Must have an Ability System Component to receive gameplay effects.
	 */
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);
	
	/**
	 * Constructs a FDamageEffectParams struct populated with values from this ability's
	 * EditDefaultsOnly properties (DamageEffectClass, DamageType, Damage, debuff settings, etc.).
	 * This factory function centralizes the creation of damage parameters, ensuring consistent
	 * initialization and reducing code duplication across different damage application scenarios.
	 * 
	 * The returned struct contains all necessary information to apply a damage effect including:
	 * - The gameplay effect class to instantiate (DamageEffectClass)
	 * - Source ability system component (from GetAbilitySystemComponentFromActorInfo())
	 * - Damage type tag and magnitude (evaluated at current ability level via Damage.GetValueAtLevel())
	 * - Debuff parameters (chance, damage, frequency, duration)
	 * - Optional target actor for additional context
	 * 
	 * This method is const-qualified because it only reads member variables without modifying
	 * the ability's state. It can be called safely from other const methods or contexts where
	 * the ability instance should not be modified.
	 * 
	 * @param TargetActor Optional actor reference to include in the damage parameters. Defaults to nullptr
	 *                    if not provided. This can be used by damage execution calculations or attribute
	 *                    capture to query target-specific information (level, resistances, etc.).
	 * @return FDamageEffectParams struct containing all damage configuration from this ability's defaults,
	 *         ready to be used with UFoxAbilitySystemLibrary::ApplyDamageEffect() or similar functions.
	 *         
	 * This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	 */
	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr) const;
	
	
	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;

protected:
	
	// Class of the GameplayEffect to apply upon impact
	// Value is configured in Blueprint subclasses via EditDefaultsOnly.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	/**
	 * Gameplay tag that identifies the type of damage this ability deals (e.g., Fire, Lightning, Physical, Arcane).
	 * This tag is used with the SetByCaller system to associate the damage magnitude with its type
	 * when applying damage effects to targets. The corresponding gameplay effect must be configured
	 * to read this tag via GetSetByCallerMagnitude() to retrieve the damage value.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag DamageType;

	/**
	 * Scalable damage value that can be configured with a base amount and optional curve table
	 * to scale damage based on ability level. When the ability is activated, GetValueAtLevel()
	 * evaluates the curve at the current ability level to calculate the final damage amount.
	 * This allows designers to create damage progression curves without code changes.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FScalableFloat Damage;
	
	/**
	 * Probability (as a percentage) that this ability will apply its associated debuff effect
	 * to the target upon dealing damage. For example, a value of 20.f means a 20% chance.
	 * The debuff type is determined by the DamageType tag (e.g., Fire damage applies Burn debuff).
	 * This value is used with the SetByCaller system via the Debuff.Chance tag when applying
	 * the damage effect to targets.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffChance = 20.f;

	/**
	 * Amount of damage dealt per tick when the debuff effect is active on a target.
	 * This is the periodic damage value applied at each DebuffFrequency interval for
	 * the duration of the debuff. For example, with DebuffDamage = 5.f and DebuffFrequency = 1.f,
	 * the target takes 5 damage every second while debuffed.
	 * This value is used with the SetByCaller system via the Debuff.Damage tag.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffDamage = 5.f;

	/**
	 * Time interval (in seconds) between each application of debuff damage.
	 * Determines how often the DebuffDamage is applied to the target while the debuff is active.
	 * For example, a value of 1.f means damage is applied every second, while 0.5f would apply
	 * damage twice per second. Combined with DebuffDuration, this determines the total number
	 * of damage ticks (Duration / Frequency).
	 * This value is used with the SetByCaller system via the Debuff.Frequency tag.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffFrequency = 1.f;

	/**
	 * Total duration (in seconds) that the debuff effect persists on the target.
	 * Determines how long the periodic damage continues after the debuff is successfully applied.
	 * For example, with DebuffDuration = 5.f and DebuffFrequency = 1.f, the debuff will deal
	 * damage 5 times over 5 seconds before expiring.
	 * This value is used with the SetByCaller system via the Debuff.Duration tag.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffDuration = 5.f;
	
	/**
	 * Magnitude of the physics impulse applied to the target actor's mesh upon death.
	 * 
	 * When a character dies from this damage, this value determines the strength of the physical
	 * force applied to their physics-simulated mesh, creating a ragdoll "launch" effect. The impulse
	 * is typically applied in the direction from the damage source to the target, making enemies
	 * fly backward when killed.
	 * 
	 * Value is configured in Blueprint subclasses via EditDefaultsOnly. However, we set a default value of 1000 here
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DeathImpulseMagnitude = 1000.f;
	
	/**
	 * Magnitude of the physics force applied to knock back the target actor when damage is dealt.
	 * 
	 * This value represents the scalar strength of the knockback force that displaces a living
	 * character when they take damage (as opposed to DeathImpulseMagnitude which only applies on death).
	 * The knockback effect pushes the target away from the damage source, creating visible impact
	 * feedback and potentially interrupting their actions.
	 * 
	 * The actual knockback is determined by multiplying this magnitude with a directional vector
	 * (typically from attacker to target) and is subject to the target's mass, physics settings,
	 * and whether the knockback chance succeeds. Higher values create more dramatic displacement.
	 * 
	 * TYPICAL VALUES:
	 * - Light attacks: 100.f - 300.f (minimal displacement)
	 * - Medium attacks: 300.f - 600.f (noticeable pushback)
	 * - Heavy attacks: 600.f - 1000.f+ (significant displacement)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackForceMagnitude = 1000.f;

	// The percentage chance that a knockback will occur when damage is dealt
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackChance = 0.f;
	
	// Returns a random TaggedMontage from an array of them
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const;
};