#pragma once

#include "GameplayEffectTypes.h"

/**
 * The following include statement was used in the tutorial. We believe it is unnecessary. We leave it here commented out
 * for documentation purposes, and in case problems arise in the future.
 * 
 * NOTE: The following include is NOT needed and should be REMOVED.
 * 
 * REASONS WHY THIS INCLUDE IS UNNECESSARY:
 * 
 * 1. WRONG LANGUAGE STANDARD:
 *    stdbool.h is a C99 standard header that defines bool, true, and false for C language.
 *    This project uses C++20, which has native bool type built into the language since C++98.
 *    C++ does not need and should not use stdbool.h.
 * 
 * 2. ABSOLUTE PATH ANTI-PATTERN:
 *    The include uses an absolute hard-coded path to a specific Visual Studio installation.
 *    This breaks portability - the code will not compile on other machines with different
 *    VS installation paths, different VS versions, or non-Windows platforms (Mac/Linux).
 *    Unreal Engine is cross-platform and this violates that principle.
 * 
 * 3. NOT USED IN THIS FILE:
 *    This header file uses bool extensively (bIsBlockedHit, bIsCriticalHit, etc.) but these
 *    work perfectly without stdbool.h because C++ has native bool support. No symbols from
 *    stdbool.h are actually being used.
 * 
 * 4. UNREAL ENGINE CONVENTIONS:
 *    Unreal Engine code never includes stdbool.h. UE uses C++ bool natively throughout the
 *    entire codebase. Including C standard library headers in UE C++ code is non-standard
 *    and can cause conflicts with UE's type system.
 * 
 * RECOMMENDATION: Delete this line entirely. The code will compile and function identically.
 */
//#include "../../../../../Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.29.30133/INCLUDE/stdbool.h"

// We created this file manually and had to add the following line as well as the '#pragma once' at the top 
#include "FoxAbilityTypes.generated.h"

class UGameplayEffect;

/**
 * Parameter container struct for configuring and applying damage gameplay effects.
 * 
 * This struct encapsulates all necessary data required to create, configure, and apply
 * a damage gameplay effect from a source actor to a target actor. It is primarily used
 * by UFoxDamageGameplayAbility and its subclasses to pass damage parameters to gameplay
 * effect application functions.
 * 
 * KEY PURPOSES:
 * 1. Damage Configuration: Holds base damage amount, damage type tag, and ability level
 *    for calculating final damage values using scalable floats and modifiers.
 * 
 * 2. Debuff Parameters: Contains complete debuff configuration (chance, damage, duration,
 *    frequency) that is passed to the damage gameplay effect via SetByCaller magnitudes.
 * 
 * 3. Context Provision: Stores both source and target ability system components along with
 *    world context, providing all necessary references for gameplay effect spec creation
 *    and application.
 * 
 * 4. Effect Class Reference: Holds the gameplay effect class to instantiate, allowing
 *    different abilities to use different damage effect configurations while sharing
 *    the same application logic.
 * 
 * This design pattern separates damage configuration data from damage application logic,
 * making the system more maintainable, testable, and allowing easy addition of new damage
 * parameters without modifying function signatures.
 */
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()
	
	/**
	 * Default constructor that initializes all member variables to their default values.
	 * Pointers are set to nullptr, numeric values to 0, and tags to empty.
	 */
	FDamageEffectParams(){}

	/**
	 * World context object required for certain gameplay effect operations and static helper functions.
	 * Typically set to the ability system component's owner or the ability instance itself.
	 * Used to provide world context when creating and applying gameplay effect specs.
	 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	/**
	 * The gameplay effect class to instantiate and apply when dealing damage.
	 * This class should contain modifiers for damage calculation, debuff application,
	 * and any other effects that should occur when damage is dealt. The effect will be
	 * configured using SetByCaller magnitudes for dynamic damage values.
	 */
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	/**
	 * Ability system component of the actor initiating the damage (the attacker/instigator).
	 * Used to provide context about the damage source, including attributes, tags, and
	 * ability level that may affect damage calculation. This is the "EffectCauser" in
	 * the gameplay effect context.
	 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	/**
	 * Ability system component of the actor receiving the damage (the target/victim).
	 * The damage gameplay effect will be applied to this component, which will process
	 * the effect using its attributes (armor, resistances) and apply the final damage
	 * and any associated debuffs.
	 */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	/**
	 * The base damage amount before any modifications from attributes, resistances, or buffs.
	 * This value is typically calculated from the ability's Damage scalable float at the current
	 * ability level, then passed to the gameplay effect via SetByCaller using the DamageType tag.
	 */
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;
	
	/**
	 * The level at which the ability is being executed, used to scale damage and other effect magnitudes.
	 * Scalable floats (like Damage) use this level to evaluate their curve tables and determine final values.
	 * For example, a level 5 ability might deal more damage than the same ability at level 1.
	 */
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;

	/**
	 * Gameplay tag that identifies the type of damage being dealt (e.g., Fire, Lightning, Physical, Arcane).
	 * This tag is used with the SetByCaller system to associate the damage magnitude with its damage type
	 * when applying the damage gameplay effect. The tag also determines which debuff type may be applied
	 * (e.g., Fire damage can apply Burn debuff, Lightning damage can apply Stun debuff).
	 */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	/**
	 * Probability (as a percentage) that the damage effect will apply its associated debuff to the target.
	 * For example, a value of 20.f represents a 20% chance to apply the debuff on hit.
	 * This value is passed to the damage gameplay effect via SetByCaller using the Debuff.Chance tag,
	 * where it is evaluated to determine if the debuff should be applied based on a random roll.
	 */
	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f;

	/**
	 * Amount of damage dealt per tick when the debuff effect is active on the target.
	 * This is the periodic damage value applied at each DebuffFrequency interval for the
	 * duration of the debuff. For example, with DebuffDamage = 5.f and DebuffFrequency = 1.f,
	 * the target takes 5 damage every second while debuffed. This value is passed to the
	 * damage gameplay effect via SetByCaller using the Debuff.Damage tag.
	 */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f;

	/**
	 * Total duration (in seconds) that the debuff effect persists on the target after being applied.
	 * Determines how long the periodic damage continues, working in conjunction with DebuffFrequency
	 * to calculate total damage ticks (Duration / Frequency). For example, with DebuffDuration = 5.f
	 * and DebuffFrequency = 1.f, the debuff will deal damage 5 times over 5 seconds before expiring.
	 * This value is passed to the damage gameplay effect via SetByCaller using the Debuff.Duration tag.
	 */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f;

	/**
	 * Time interval (in seconds) between each application of debuff damage to the target.
	 * Determines how often DebuffDamage is applied while the debuff is active. For example,
	 * a value of 1.f means damage is applied every second, while 0.5f applies damage twice per second.
	 * Combined with DebuffDuration, this determines the total number of damage ticks. This value is
	 * passed to the damage gameplay effect via SetByCaller using the Debuff.Frequency tag.
	 */
	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f;
	
	/**
	 * Magnitude of the physics impulse applied to the target actor's mesh upon death.
	 * 
	 * When a character dies from this damage, this value determines the strength of the physical
	 * force applied to their physics-simulated mesh, creating a ragdoll "launch" effect. The impulse
	 * is typically applied in the direction from the damage source to the target, making enemies
	 * fly backward when killed.
	 */
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f;
	
	/**
	 * The directional physics impulse vector applied to the target actor's mesh upon death.
	 * 
	 * This vector defines both the direction and magnitude of the physical force applied to a
	 * character's physics-simulated mesh when they die from this damage. Unlike DeathImpulseMagnitude
	 * which only stores the scalar strength, this FVector encodes the complete impulse with:
	 * 
	 * - DIRECTION: The unit vector pointing from damage source to target (or custom direction)
	 * - MAGNITUDE: The length of the vector, typically equal to DeathImpulseMagnitude
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;
	
	/**
	 * Magnitude of the physics force applied to knock back the target actor when damage is dealt.
	 * 
	 * This value represents the scalar strength of the knockback force that displaces a living
	 * character when they take damage (as opposed to DeathImpulseMagnitude which only applies on death).
	 * The knockback effect pushes the target away from the damage source, creating visible impact
	 * feedback and potentially interrupting their actions.
	 * 
	 * The actual knockback (KnockbackForce) is determined by multiplying this magnitude with a directional vector
	 * (typically from attacker to target) and is subject to the target's mass, physics settings,
	 * and whether the knockback chance succeeds. Higher values create more dramatic displacement.
	 * 
	 * TYPICAL VALUES:
	 * - Light attacks: 100.f - 300.f (minimal displacement)
	 * - Medium attacks: 300.f - 600.f (noticeable pushback)
	 * - Heavy attacks: 600.f - 1000.f+ (significant displacement)
	 */
	UPROPERTY(BlueprintReadWrite)
	float KnockbackForceMagnitude = 0.f;
	
	// Probability (as a percentage) that the damage effect will apply knockback force to the target.
	UPROPERTY(BlueprintReadWrite)
	float KnockbackChance = 0.f;

	/**
	 * The directional physics force vector applied to knock back the target actor when damage is dealt.
	 * 
	 * This vector defines both the direction and magnitude of the physical force that displaces
	 * a living character when they take damage (unlike DeathImpulse which only applies on death).
	 * This value is set in FoxDamageGameplayAbility.cpp by muliplying the direction vector we wish to use
	 * by the KnockbackForceMagnitude
	 * 
	 * VECTOR COMPONENTS:
	 * - DIRECTION: Typically the normalized vector from damage source to target, determining which
	 *              way the target is pushed. Can be customized for special effects (e.g., always
	 *              knock upward, or in ability-specific directions like whirlwind patterns).
	 * - MAGNITUDE: The length of the vector, usually equal to KnockbackForceMagnitude, determining
	 *              how far the target is displaced. The actual displacement also depends on the
	 *              target's mass and physics settings
	 */
	UPROPERTY(BlueprintReadWrite)
	FVector KnockbackForce = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FFoxGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
	
public:
	
	// Functions that get the values of the bIsCriticalHit and bIsBlockedHit member variables
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit () const { return bIsBlockedHit; }
	
	// Returns whether a debuff was successfully applied during this gameplay effect context
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }
	
	// Returns the amount of damage dealt per tick when the debuff effect is active
	float GetDebuffDamage() const { return DebuffDamage; }
	
	// Returns the total duration (in seconds) that the debuff effect persists on the target
	float GetDebuffDuration() const { return DebuffDuration; }
	
	// Returns the time interval (in seconds) between each application of debuff damage
	float GetDebuffFrequency() const { return DebuffFrequency; }
	
	// Returns a shared pointer to the gameplay tag identifying the type of damage dealt
	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }
	
	// Returns the DeathImpulse vector which is the directional physics impulse vector applied to the target actor's 
	// mesh upon death.
	FVector GetDeathImpulse() const { return DeathImpulse; }
	
	// Returns the KnockbackForce this vector defines both the direction and magnitude of the physical force that displaces
	// a living character when they take damage (unlike DeathImpulse which only applies on death).
	FVector GetKnockbackForce() const { return KnockbackForce; }

	// Functions that Set the values of the bIsCriticalHit and bIsBlockedHit member variables
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	
	// Sets whether a debuff was successfully applied during this gameplay effect context
	void SetIsSuccessfulDebuff(bool bInIsDebuff) { bIsSuccessfulDebuff = bInIsDebuff; }

	// Sets the amount of damage dealt per tick when the debuff effect is active on the target
	void SetDebuffDamage(float InDamage) { DebuffDamage = InDamage; }

	// Sets the total duration (in seconds) that the debuff effect persists on the target after being applied
	void SetDebuffDuration(float InDuration) { DebuffDuration = InDuration; }

	// Sets the time interval (in seconds) between each application of debuff damage to the target
	void SetDebuffFrequency(float InFrequency) { DebuffFrequency = InFrequency; }
	
	/**
	 * Sets the gameplay tag that identifies the type of damage dealt in this effect context.
	 * 
	 * This function assigns a shared pointer to a FGameplayTag that categorizes the damage type
	 * (e.g., "Damage.Fire", "Damage.Lightning", "Damage.Physical", "Damage.Arcane"). The damage
	 * type tag is used throughout the damage calculation pipeline to:
	 * 
	 * 1. Determine which resistance attribute to apply (e.g., Fire Resistance vs Lightning Resistance)
	 * 2. Select the appropriate debuff to apply (e.g., Burn for Fire, Stun for Lightning)
	 * 3. Trigger type-specific visual and audio effects
	 * 
	 * The method accepts a TSharedPtr to enable efficient memory management when the same damage
	 * type tag is shared across multiple effect contexts (common in AoE abilities or replicated effects).
	 * 
	 * USAGE EXAMPLE:
	 * When a fireball ability creates a damage effect context, it calls:
	 *   Context.SetDamageType(MakeShared<FGameplayTag>(FGameplayTag::RequestGameplayTag("Damage.Fire")));
	 * 
	 * This ensures all damage calculations, resistance checks, and debuff applications use the
	 * Fire damage type throughout the effect's lifetime.
	 * 
	 * @param InDamageType Shared pointer to the gameplay tag identifying the damage type. This pointer
	 *                     may be shared with other contexts to avoid redundant tag allocations. Can be
	 *                     null if damage type is irrelevant for the specific effect (e.g., true damage).
	 */
	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; }
	
	// Sets the value of the DeathImpulse vector which is the directional physics impulse vector applied to the target actor's 
	// mesh upon death.
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; }
	
	// Returns the KnockbackForce this vector defines both the direction and magnitude of the physical force that displaces
	// a living character when they take damage (unlike DeathImpulse which only applies on death).
	void SetKnockbackForce(const FVector& InForce) { KnockbackForce = InForce; }
	
	// Copied from the class we are inheriting from
	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return StaticStruct();
	}
	
	// Copied from the class we are inheriting from
	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FFoxGameplayEffectContext* Duplicate() const
	{
		FFoxGameplayEffectContext* NewContext = new FFoxGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	
protected:
	
	// If true a blocked hit has occured
	UPROPERTY()
	bool bIsBlockedHit = false;
	
	// If true a critical hit has occured
	UPROPERTY()
	bool bIsCriticalHit = false;
	
	/**
	 * Flag indicating whether a debuff was successfully applied during this gameplay effect context.
	 * Set to true when the debuff application succeeds (typically after a successful random roll
	 * against DebuffChance), and false otherwise. This is used to track debuff application status
	 * for gameplay logic and is replicated over the network.
	 */
	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	/**
	 * Amount of damage dealt per tick when the debuff effect is active on the target.
	 * This value represents the periodic damage applied at each DebuffFrequency interval
	 * for the duration specified by DebuffDuration. For example, with DebuffDamage = 5.f
	 * and DebuffFrequency = 1.f, the target takes 5 damage every second while debuffed.
	 * This is replicated over the network to ensure consistent debuff behavior.
	 */
	UPROPERTY()
	float DebuffDamage = 0.f;

	/**
	 * Total duration (in seconds) that the debuff effect persists on the target after being applied.
	 * Determines how long the periodic damage continues, working in conjunction with DebuffFrequency
	 * to calculate the total number of damage ticks (Duration / Frequency). For example, with
	 * DebuffDuration = 5.f and DebuffFrequency = 1.f, the debuff will deal damage 5 times over
	 * 5 seconds before expiring. This is replicated over the network.
	 */
	UPROPERTY()
	float DebuffDuration = 0.f;

	/**
	 * Time interval (in seconds) between each application of debuff damage to the target.
	 * Determines how often DebuffDamage is applied while the debuff is active. For example,
	 * a value of 1.f means damage is applied every second, while 0.5f applies damage twice
	 * per second. Combined with DebuffDuration, this determines the total number of damage
	 * ticks. This is replicated over the network.
	 */
	UPROPERTY()
	float DebuffFrequency = 0.f;

	/**
	 * Shared pointer to a gameplay tag that identifies the type of damage dealt (e.g., Fire, Lightning, Physical).
	 * 
	 * WHAT IS TSharedPtr?
	 * TSharedPtr is Unreal Engine's implementation of a reference-counted smart pointer, similar to std::shared_ptr
	 * in standard C++. It automatically manages the lifetime of the object it points to by keeping track of how
	 * many TSharedPtr instances reference the same object. When the last TSharedPtr referencing an object is
	 * destroyed, the object is automatically deleted. This prevents memory leaks and dangling pointers.
	 * 
	 * Key characteristics of TSharedPtr:
	 * - Multiple TSharedPtr instances can share ownership of the same object
	 * - Thread-safe reference counting (atomic operations)
	 * - Automatic memory management (no manual delete required)
	 * - Can be null-checked with IsValid()
	 * - Supports dereferencing with * and -> operators
	 * 
	 * WHY NO UPROPERTY MACRO?
	 * TSharedPtr cannot be marked with UPROPERTY because Unreal's reflection system (UPROPERTY) is designed
	 * to work with UObject-based pointers and primitive types that can be directly serialized. TSharedPtr
	 * uses custom memory management and reference counting that is incompatible with Unreal's property system.
	 * 
	 * Instead, we handle serialization manually in the NetSerialize() function. When saving, we check if
	 * DamageType.IsValid() and set a bit in RepBits. When loading, we allocate a new FGameplayTag if needed
	 * and call NetSerialize on the pointed-to object. This gives us fine-grained control over network
	 * replication while still benefiting from shared pointer semantics for memory management.
	 * 
	 * WHY USE TSharedPtr FOR DAMAGE TYPE?
	 * We use TSharedPtr because the same FGameplayTag object may be shared across multiple FFoxGameplayEffectContext
	 * instances and we want to avoid unnecessary copies while ensuring the tag remains valid for the lifetime of
	 * all contexts that reference it.
	 * 
	 * CONCRETE EXAMPLES OF MULTIPLE CONTEXTS:
	 * 
	 * 1. Area of Effect Abilities:
	 *    When a fireball explodes and hits 5 enemies, the ability creates 5 separate FFoxGameplayEffectContext
	 *    instances (one per target). All 5 contexts can share the same "Damage.Fire" tag via TSharedPtr rather
	 *    than allocating 5 separate FGameplayTag objects. This saves memory and improves performance.
	 * 
	 * 2. Replicated Gameplay Effects:
	 *    When a damage effect is applied on the server and replicated to multiple clients, each machine creates
	 *    its own FFoxGameplayEffectContext via NetSerialize(). The original context on the server, the replicated
	 *    context on each client, and any duplicates created during execution can all share the same DamageType
	 *    tag through shared ownership, avoiding redundant allocations.
	 * 
	 * 3. Effect Duplication:
	 *    When FFoxGameplayEffectContext::Duplicate() is called to create a modified copy of a context (common
	 *    during gameplay effect execution), the duplicated context shares the same DamageType tag with the
	 *    original. This is both memory-efficient and semantically correct since the damage type doesn't change
	 *    between the original and duplicate contexts.
	 * 
	 * 4. Debuff Application Chains:
	 *    When a damage effect successfully applies a debuff, it may trigger additional gameplay effects (like
	 *    visual effects or sound cues) that reference the original damage context. All these derived contexts
	 *    can share the same DamageType tag, ensuring consistency across the effect chain while minimizing
	 *    memory overhead.
	 * 
	 * The shared ownership model is ideal for tags that may be referenced by multiple systems simultaneously,
	 * providing efficient memory usage without risking premature deletion or dangling references.
	 */
	TSharedPtr<FGameplayTag> DamageType;
	
	/**
	 * The directional physics impulse vector applied to the target actor's mesh upon death.
	 * 
	 * This vector defines both the direction and magnitude of the physical force applied to a
	 * character's physics-simulated mesh when they die from this damage. Unlike DeathImpulseMagnitude
	 * which only stores the scalar strength, this FVector encodes the complete impulse with:
	 * 
	 * - DIRECTION: The unit vector pointing from damage source to target (or custom direction)
	 * - MAGNITUDE: The length of the vector, typically equal to DeathImpulseMagnitude
	*/
	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;
	
	/**
	 * The directional physics force vector applied to knock back the target actor when damage is dealt.
	 * 
	 * This vector defines both the direction and magnitude of the physical force that displaces
	 * a living character when they take damage (unlike DeathImpulse which only applies on death).
	 * This value is set in FoxDamageGameplayAbility.cpp by muliplying the direction vector we wish to use
	 * by the KnockbackForceMagnitude
	 * 
	 * VECTOR COMPONENTS:
	 * - DIRECTION: Typically the normalized vector from damage source to target, determining which
	 *              way the target is pushed. Can be customized for special effects (e.g., always
	 *              knock upward, or in ability-specific directions like whirlwind patterns).
	 * - MAGNITUDE: The length of the vector, usually equal to KnockbackForceMagnitude, determining
	 *              how far the target is displaced. The actual displacement also depends on the
	 *              target's mass and physics settings
	 */
	UPROPERTY()
	FVector KnockbackForce = FVector::ZeroVector;
};

/**
 * Template specialization that defines type traits for FFoxGameplayEffectContext.
 * 
*  * WHAT THIS MEANS:
 * 1. "Template Specialization": Think of 'TStructOpsTypeTraits' as a generic "Rules Manual" for 
 *    how Unreal handles structs. By adding '<FFoxGameplayEffectContext>', we are creating a 
 *    SPECIAL version of that manual specifically for our struct.
 * 
 * 2. "Type Traits": These are "features" or "abilities" we are telling the engine our struct has.
 *    Instead of the engine guessing how to copy or send our struct over the network, these 
 *    traits act as a "spec sheet" that explicitly tells the engine: "Hey, we have a custom 
 *    NetSerialize and a custom Duplicate function—please use them!"
 * 
 * Without this "manual," Unreal would treat our struct like a simple block of data (a POD), 
 * ignoring our custom logic and potentially crashing or losing data during replication.
 * 
 * This struct tells Unreal Engine how to handle FFoxGameplayEffectContext during various operations like
 * serialization and copying. It inherits from TStructOpsTypeTraitsBase2 which provides default implementations
 * for common struct operations.
 * 
 * The enum flags control specific behaviors:
 * 
 * - WithNetSerializer = true:
 *   Indicates that FFoxGameplayEffectContext has a custom NetSerialize() function for network replication.
 *   When true, Unreal will call our overridden NetSerialize() method (defined in FoxAbilityTypes.cpp) instead
 *   of using the default property-based serialization. This is necessary because we have custom boolean flags
 *   (bIsBlockedHit and bIsCriticalHit) that need special handling during network replication to minimize
 *   bandwidth by using bit packing.
 * 
 * - WithCopy = true:
 *   Indicates that FFoxGameplayEffectContext has a custom copy implementation via the Duplicate() function.
 *   When true, Unreal will use our overridden Duplicate() method for creating copies of this context instead
 *   of performing a simple memcpy. This is important because FFoxGameplayEffectContext contains complex members
 *   like TSharedPtr (HitResult) that require proper deep copying to avoid shared pointer issues and ensure
 *   each copy has its own independent data.
 * 
 * Without this trait struct, Unreal would not know to use our custom serialization and copy logic, which could
 * lead to incorrect network replication and shallow copies that share data between contexts.
 */
/**
 * THE 'template<>' SYNTAX: FULL TEMPLATE SPECIALIZATION
 * 
 * 1. THE PRIMARY TEMPLATE (The General Rule):
 *    In the Engine source code, Unreal defines a "Primary Template." It looks 
 *    essentially like this:
 * 
 *    template<typename T>
 *    struct TStructOpsTypeTraits
 *    {
 *        enum { WithNetSerializer = false, WithCopy = false };
 *    };
 * 
 *    This is a "generic blueprint" meant to work for any struct (T). By default, 
 *    it tells the compiler that structs are basic and do not have custom logic 
 *    for networking or copying.
 * 
 * 2. FULL SPECIALIZATION (The 'template<>'):
 *    The 'template<>' syntax tells the C++ compiler: "I am not creating a new 
 *    template. I am providing a specific, concrete implementation for a 
 *    previously declared template."
 * 
 *    - Why are the brackets empty? 
 *      In the Primary Template (template<typename T>), 'T' is a variable 
 *      waiting to be filled. In this block, we have already "filled in" that 
 *      variable by providing <FFoxGameplayEffectContext> in the struct name. 
 *      Since there are no more template variables left to define for this 
 *      specific version, the parameter list is empty: <>.
 * 
 * 3. COMPILER DISPATCH (How the Engine uses this):
 *    When the Unreal Engine compiles its networking or serialization code, 
 *    it "asks" the compiler: "What are the traits for FFoxGameplayEffectContext?"
 * 
 *    The compiler sees two options:
 *    A) The generic "Primary Template" (The "General Rule").
 *    B) This "Full Specialization" (The "Specific Exception").
 * 
 *    According to C++ language rules, the compiler must always choose the 
 *    most specific version available. Because this version is explicitly 
 *    written for FFoxGameplayEffectContext, it "overrides" the generic version, 
 *    allowing our custom WithNetSerializer and WithCopy flags to take effect.
 */
template<>
struct TStructOpsTypeTraits<FFoxGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FFoxGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
