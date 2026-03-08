// Copyright TryingToMakeGames


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "FoxAbilityTypes.h"
#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"


/**
 * Static helper struct that manages attribute capture definitions for the damage calculation system.
 * 
 * This struct serves as a centralized registry of all attributes that need to be captured during damage
 * calculations in UExecCalc_Damage.
 * The struct captures both secondary attributes (Armor, BlockChance, CriticalHit stats, ArmorPenetration) and
 * resistance attributes (Fire, Lightning, Arcane, Physical) from either the Source (attacker) or Target (defender)
 * of a gameplay effect, allowing the damage calculation to read and use these values when computing final damage.
 */
struct FoxDamageStatics
{
	/*
	* DECLARE_ATTRIBUTE_CAPTUREDEF is an Unreal Engine macro that creates two member variables for attribute capture:
	* 
	* 1. ArmorProperty - An FProperty pointer that references the Armor attribute's reflection metadata in UFoxAttributeSet.
	*    
	*    "Reflection metadata" refers to the runtime type information that Unreal's reflection system generates
	*    and stores for any UPROPERTY-marked variable. This metadata allows the engine to
	*    find and access variables by name at runtime (even on unknown object instances)
	*    
	*    The ArmorProperty pointer specifically points to the reflection metadata for the "Armor" float variable
	*    inside UFoxAttributeSet. This pointer acts as a "blueprint" or "map" that tells the damage calculation
	*    system exactly where and how to find the Armor value on any character's attribute set instance, even
	*    though we don't know which specific character object we'll be reading from at compile time.
	*    
	*    This is essential for the attribute capture system because it needs to dynamically read attribute values
	*    from different characters (source attacker and target defender) during gameplay effect execution, without
	*    hardcoding specific object references or using brittle string-based lookups.
	* 
	* 2. ArmorDef - An FGameplayEffectAttributeCaptureDefinition that specifies how to capture the attribute (from Source/Target, snapshot vs live)
	* 
	* These declarations enable the damage calculation system to read attribute values from characters during gameplay
	* effect execution. The actual initialization of these variables happens in the FoxDamageStatics constructor below
	* using DEFINE_ATTRIBUTE_CAPTUREDEF.
	* 
	* The following DECLARE_ATTRIBUTE_CAPTUREDEF lines create similar pairs of variables for other attributes used in
	* damage calculations (e.g., BlockChanceProperty/BlockChanceDef, ArmorPenetrationProperty/ArmorPenetrationDef, etc.)
	*/
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	
	// Constructor of this struct
	FoxDamageStatics()
	{
		
		/* 
		  DEFINE_ATTRIBUTE_CAPTUREDEF is an Unreal Engine macro that performs two critical initializations
		  to set up attribute capture for gameplay effect calculations:
		
		  1. Initializes ArmorProperty (an FProperty pointer):
			 - Uses Unreal's reflection system via FindFieldChecked to search UFoxAttributeSet's class definition
			 - Locates the UPROPERTY-marked float variable named "Armor" within the attribute set class
			 - Stores a pointer to that property's reflection information (FProperty*)
			 - This pointer acts as a "blueprint" that tells the system where to find and how to read the Armor
			   value from any UFoxAttributeSet instance at runtime, without needing to know the specific object
			 - The "Checked" suffix means the engine crashes immediately at startup if the property isn't found,
			   providing early detection of typos or missing attributes during development
		
		  2. Initializes ArmorDef (an FGameplayEffectAttributeCaptureDefinition):
			 - Wraps the ArmorProperty pointer with additional capture configuration
			 - Specifies FROM WHERE to capture: Target (the defender receiving the effect, not the Source attacker)
			 - Specifies WHEN to capture: false = live value (read current value during execution, not a snapshot
			   taken when the effect was created)
			 - This capture definition can then be registered with the execution calculation's
			   RelevantAttributesToCapture array and used with AttemptCalculateCapturedAttributeMagnitude to
			   retrieve the actual attribute value during damage calculations
		
		  Macro Parameters:
		  - UFoxAttributeSet: The attribute set class that contains the attribute to capture
		  - Armor: The name of the attribute variable to capture (must match UPROPERTY name exactly)
		  - Target: Specifies to capture from the effect's Target (could also be Source for attacker attributes)
		  - false: Don't snapshot - capture the live/current value during execution (true would snapshot at creation)
		*/
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, BlockChance, Target, false);
		
		// This one captures the Source's ArmorPenetration attribute because it is the attackers armor penetration
		// that we want to use in the calculation below
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, CriticalHitDamage, Source, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UFoxAttributeSet, PhysicalResistance, Target, false);
	}
};

// Function that creates a static instance (DStatics) of FoxDamageStatics struct and returns a reference to it.
// This instance does not get destroyed until the program exits and this same instance is returned every time 
// this function is called. This is a singleton pattern for the FoxDamageStatics struct.
static const FoxDamageStatics& DamageStatics()
{
	static FoxDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	/*
	RelevantAttributesToCapture is a TArray<FGameplayEffectAttributeCaptureDefinition> member variable inherited from
	the UGameplayEffectExecutionCalculation base class. It serves as a registration list that tells Unreal's Gameplay
	Ability System which attributes this execution calculation needs to read from characters during damage calculations.

	This array must be populated in the constructor (before any Execute_Implementation calls occur) so the GAS can:
	1. Pre-allocate memory and set up capture infrastructure for the specified attributes
	2. Validate that all requested attributes exist and are accessible on the target attribute sets
	3. Enable the AttemptCalculateCapturedAttributeMagnitude() function to successfully retrieve attribute values
	   during Execute_Implementation by looking up the registered capture definitions

	Each FGameplayEffectAttributeCaptureDefinition stored in this array contains:
	- A pointer to the attribute set that contains the attribute (e.g., UFoxAttributeSet)
	- An FProperty pointer identifying which attribute to capture (e.g., ArmorProperty points to the Armor float in UFoxAttributeSet)
	- A source specification (EGameplayEffectAttributeCaptureSource: Source attacker or Target defender)
	- A snapshot flag (bool: capture value at effect creation time vs live value during execution)

	Without registering capture definitions here, AttemptCalculateCapturedAttributeMagnitude() calls in Execute_Implementation
	would fail silently and return default values (usually 0.f), causing incorrect damage calculations.

	The following Add() calls register all attributes needed for this damage calculation system:
	- Secondary attributes: Armor, BlockChance, ArmorPenetration, CriticalHitChance, CriticalHitResistance, CriticalHitDamage
	- Resistance attributes: FireResistance, LightningResistance, ArcaneResistance, PhysicalResistance

	Each definition was initialized in the FoxDamageStatics constructor using DEFINE_ATTRIBUTE_CAPTUREDEF and is
	accessed here through the DamageStatics() function that returns the singleton instance of the FoxDamageStatics struct
	(e.g., DamageStatics().ArmorDef).
	*/
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluationParameters,
	const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	// Retrieves the singleton instance of FFoxGameplayTags, which contains all gameplay tag definitions and mappings
	// used throughout the game, including the DamageTypesToDebuffs map that associates damage types with their
	// corresponding debuff effects (e.g., Fire damage applies Burn debuff, Lightning damage applies Stun debuff).
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	// Iterates through all entries in the DamageTypesToDebuffs map to check each damage type that was dealt and
	// determine if its corresponding debuff should be applied. The map contains key-value pairs where the key is
	// a damage type tag (e.g., Damage.Fire, Damage.Lightning) and the value is the associated debuff type tag
	// (e.g., Debuff.Burn, Debuff.Stun).
	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		// Extracts the damage type tag (the key) from the current map pair, which identifies what type of damage 
		// is being dealt (e.g., Damage.Fire, Damage.Lightning, Damage.Arcane, Damage.Physical)
		const FGameplayTag& DamageType = Pair.Key;

		// Extracts the debuff type tag (the value) from the current map pair, which identifies what debuff should 
		// be applied for this damage type (e.g., Debuff.Burn for Fire damage, Debuff.Stun for Lightning damage)
		const FGameplayTag& DebuffType = Pair.Value;
		
		// Retrieves the damage magnitude for the current damage type from the gameplay effect spec.
		// Earlier, when creating the spec, SetSetByCallerMagnitude() was called with a tag (key) and damage value.
		// Here we use GetSetByCallerMagnitude() with the DamageType tag (key) to retrieve that stored damage value.
		// The second parameter (false) suppresses warnings if the tag is not found, since not all damage types may be
		// present on every attack. The third parameter (-1.f) is the default value returned if this damage type was
		// not set on the spec, which we use below to determine if this damage type is actually being dealt.
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);

		// Checks if the damage value is greater than -0.5 to determine if this damage type was actually set on the spec.
		// We use -0.5 instead of 1.0 as the threshold to account for floating-point precision issues, since unset values
		// return -1.f. Floating point (im)percision refers to the fact that a value of -1 might actually be stored as 
		// -0.999997, -1.000001, or a similar value. If TypeDamage is greater than -0.5, it means a valid damage value 
		// was provided and we should proceed with debuff calculations for this damage type.
		if (TypeDamage > -.5f)
		{
			// Retrieves the debuff application chance from the gameplay effect spec using the Debuff_Chance tag.
			// This value represents the base percentage chance (0-100) that a debuff will be applied when this damage
			// type hits, before being modified by the target's resistance. For example, a value of 25.f means 25% chance.
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);
			
			// Declares and initializes a float variable to hold the target's resistance value for this specific debuff type,
			// starting at 0 as a safe default before attempting to capture the actual resistance attribute value.
			float TargetDebuffResistance = 0.f;

			// Retrieves the resistance attribute tag that corresponds to this damage type from the DamageTypesToResistances map,
			// which maps each damage type (e.g., Damage.Fire) to its matching resistance attribute tag (e.g., Attributes.Resistance.Fire).
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
			
			/*
			Calculates the final effective value of the target's resistance attribute for this damage type.

			This function retrieves the attribute specified by InTagsToDefs[ResistanceTag] (e.g., FireResistance),
			evaluates all active gameplay effect modifiers on that attribute, applies any tag-based conditional modifiers
			from EvaluationParameters (e.g., "reduce resistance by 25% if target has Debuff.Weakened"), and writes
			the final calculated value into TargetDebuffResistance.

			Parameters:
			- InTagsToDefs[ResistanceTag]: Capture definition specifying which attribute to read (Target's resistance, live value)
			- EvaluationParameters: Contains SourceTags and TargetTags that enable tag-based conditional modifiers
			- TargetDebuffResistance: Output parameter that receives the final calculated resistance value

			Returns true if the attribute was successfully captured and calculated, false otherwise.
			*/
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);

			// Clamps the captured resistance value to ensure it's never negative by taking the maximum of the captured value and 0,
			// preventing invalid negative resistance values from incorrectly increasing debuff chance instead of decreasing it.
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);

			// Calculates the final effective debuff application chance by reducing the SourceDebuffChance based on target's resistance percentage,
			// where each point of resistance reduces the chance proportionally (e.g., 50% resistance halves the debuff chance from 25% to 12.5%).
			const float EffectiveDebuffChance = SourceDebuffChance * ( 100 - TargetDebuffResistance ) / 100.f;

			// Generates a random integer between 1 and 100 and compares it against EffectiveDebuffChance to determine if the debuff is applied,
			// where a random number less than the effective chance results in successful debuff application (e.g., 12.5% chance means success if roll < 12.5).
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;

			// Checks if the debuff roll was successful and should be applied to the target, entering this block to handle debuff application logic
			// such as adding gameplay tags, applying duration effects, or triggering debuff-specific visuals and mechanics.
			if (bDebuff)
			{
				// Retrieves a handle to the gameplay effect context data structure associated with this spec, which stores
				// additional metadata about the effect beyond what's in the spec itself (e.g., hit result, instigator,
				// debuff parameters). This handle allows us to access and modify custom context data through our library
				// functions, enabling the effect to communicate debuff information to other systems like UI, VFX, or
				// gameplay cues that need to know what debuff was applied and its parameters.
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

				// Marks the effect context to indicate that a debuff was successfully applied to the target, setting an
				// internal boolean flag that other systems can query to determine if this damage application included a
				// debuff component (as opposed to pure damage without status effects).
				UFoxAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);

				// Stores the specific damage type tag (e.g., Damage.Fire, Damage.Lightning) that triggered this debuff
				// in the effect context, allowing downstream systems to know which elemental type caused the debuff and
				// apply appropriate visuals, sounds, or gameplay logic based on the damage type.
				UFoxAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
				
				// Retrieves the damage-per-tick value for this debuff from the gameplay effect spec using the Debuff_Damage
				// tag as a SetByCaller key, which was set when the ability created the spec and represents how much damage
				// each periodic tick of the debuff will deal (e.g., 10 fire damage every 0.5 seconds for a Burn debuff).
				// The second parameter (false) suppresses warnings if the tag is not found, preventing log spam since not all
				// damage types may include debuff parameters. The third parameter (-1.f) is the default value returned if this
				// tag was not set on the spec, which can be used to detect uninitialized debuff parameters.
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);

				// Retrieves the total duration for this debuff from the gameplay effect spec using the Debuff_Duration tag,
				// which was set when the ability created the spec and represents how long the debuff will last on the target
				// in seconds (e.g., 5.0 seconds for a 5-second Burn effect). The second parameter (false) suppresses warnings
				// if the tag is not found, preventing log spam since not all damage types may include debuff parameters. The
				// third parameter (-1.f) is the default value returned if this tag was not set on the spec, which can be used
				// to detect uninitialized debuff parameters.
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);

				// Retrieves the tick frequency for this debuff from the gameplay effect spec using the Debuff_Frequency tag,
				// which was set when the ability created the spec and represents how often the debuff deals damage in seconds
				// (e.g., 0.5 means the debuff ticks every 0.5 seconds, or twice per second). The second parameter (false)
				// suppresses warnings if the tag is not found, preventing log spam since not all damage types may include debuff
				// parameters. The third parameter (-1.f) is the default value returned if this tag was not set on the spec,
				// which can be used to detect uninitialized debuff parameters.
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);

				// Stores the retrieved debuff damage-per-tick value into the effect context, making it accessible to other
				// systems that need to know how much damage each periodic tick should deal when applying the debuff effect.
				UFoxAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);

				// Stores the retrieved debuff duration value into the effect context, making it accessible to systems that
				// need to know how long the debuff should persist on the target before expiring.
				UFoxAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);

				// Stores the retrieved debuff tick frequency value into the effect context, making it accessible to systems
				// that need to know how often the debuff should trigger its periodic damage effect on the target.
				UFoxAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

// FGameplayEffectCustomExecutionParameters - a struct that provides context and utility functions for execution 
// calculations. It contains the owning spec, source/target ASCs, and methods to retrieve captured attribute values. 
// OutExecutionOutput is the output variable
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	/*
	Declares and initializes a map that associates gameplay tags with their corresponding attribute capture definitions.
	This map serves as a lookup table that allows the damage calculation system to dynamically retrieve the appropriate
	capture definition for any captured attribute using its gameplay tag as the key.

	For example, when processing Fire damage, we can use Tags.Attributes_Resistance_Fire as the key to retrieve
	DamageStatics().FireResistanceDef (the capture definition that specifies how to read the target's FireResistance
	attribute value). This design pattern enables the loop below to handle multiple damage types and resistances
	generically without hardcoding each attribute individually.

	The map is populated immediately below with entries for all secondary attributes (Armor, BlockChance, CriticalHit stats)
	and all resistance attributes (Fire, Lightning, Arcane, Physical), making them accessible throughout the damage
	calculation process, particularly in the DetermineDebuff function and the damage type iteration loop.
	*/
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	// Get the singleton instance of FFoxGameplayTags
	const FFoxGameplayTags& Tags = FFoxGameplayTags::Get();
	
	
	/*
	Populates the TagsToCaptureDefs map with key-value pairs that associate gameplay tags with their corresponding
	attribute capture definitions. Each Add() call follows the same pattern:
	- Key: A gameplay tag from the FFoxGameplayTags singleton that identifies a specific attribute
	  (e.g., Tags.Attributes_Secondary_Armor identifies the Armor attribute)
	- Value: The corresponding FGameplayEffectAttributeCaptureDefinition from DamageStatics() that specifies
	  how to capture that attribute (e.g., DamageStatics().ArmorDef contains the capture configuration for Armor)

	Example: TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef)
	This line adds an entry where Tags.Attributes_Secondary_Armor (the gameplay tag for Armor) maps to
	DamageStatics().ArmorDef (the capture definition that specifies: capture from Target, live value, Armor property).

	All subsequent Add() calls follow this exact same pattern for their respective attributes, building a complete
	lookup table that enables dynamic attribute retrieval throughout the damage calculation process, particularly
	in the DetermineDebuff function and the damage type iteration loop.

	Why we create this map in addition to the RelevantAttributesToCapture array populated in the constructor:

	1. RelevantAttributesToCapture (constructor):
	   - Purpose: Registers capture definitions with the Gameplay Ability System's infrastructure
	   - When: Populated once during object construction, before any Execute_Implementation calls
	   - Why: Tells GAS which attributes need capture setup, enables AttemptCalculateCapturedAttributeMagnitude to work
	   - Access pattern: Sequential array that GAS internally uses for validation and setup
	   - Example: RelevantAttributesToCapture.Add(DamageStatics().ArmorDef) registers Armor for capture

	2. TagsToCaptureDefs (here in Execute_Implementation):
	   - Purpose: Provides tag-based lookup of capture definitions during execution for dynamic/generic code
	   - When: Created fresh each time Execute_Implementation runs (per damage calculation)
	   - Why: Enables loops and helper functions to dynamically retrieve capture defs using gameplay tags as keys
	   - Access pattern: Map lookup via gameplay tag (e.g., TagsToCaptureDefs[Tags.Attributes_Resistance_Fire])
	   - Example: In the DamageTypesToResistances loop, we use ResistanceTag to look up the matching CaptureDef
	*/
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);

	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	
	// Gets the ability system components of the source and target actors from the ExecutionParams struct input parameter
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// Attempts to get the avatar actors of the source and target actors from their ability system components
	// If the ability system component is null, the avatar actor will be set to null
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	
	/*
	Initialize SourcePlayerLevel to 1 as a safe default fallback value in case the source avatar doesn't implement
	the CombatInterface or the GetPlayerLevel call fails, ensuring the damage calculation can still proceed with a
	valid level value rather than having an uninitialized variable.
	*/
	int32 SourcePlayerLevel = 1;

	/*
	Uses UCombatInterface (the U-prefixed UCLASS) to perform a runtime reflection-based check if SourceAvatar implements
	the CombatInterface. We use the U-prefixed version because Implements<>() is a reflection system function that requires
	the UObject-based interface class (generated by UINTERFACE macro) to query Unreal's type information system and determine
	if this actor has the interface in its class hierarchy.
	*/
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		/*
		Uses ICombatInterface (the I-prefixed native interface class) to call the static Execute helper function that safely
		invokes GetPlayerLevel on the source avatar. We use the I-prefixed version because Execute_GetPlayerLevel is a static
		helper function generated by Unreal for BlueprintNativeEvent functions, and it's defined in the native interface class
		(ICombatInterface), not the reflection wrapper (UCombatInterface). This Execute function handles both C++ and Blueprint
		implementations of the interface method, calling the appropriate _Implementation version.
		*/
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	/*
	Initialize TargetPlayerLevel to 1 as a safe default fallback value in case the target avatar doesn't implement
	the CombatInterface or the GetPlayerLevel call fails, ensuring the damage calculation can still proceed with a
	valid level value rather than having an uninitialized variable.
	*/
	int32 TargetPlayerLevel = 1;

	/*
	Uses UCombatInterface (the U-prefixed UCLASS) to perform a runtime reflection-based check if TargetAvatar implements
	the CombatInterface. We use the U-prefixed version because Implements<>() is a reflection system function that requires
	the UObject-based interface class (generated by UINTERFACE macro) to query Unreal's type information system and determine
	if this actor has the interface in its class hierarchy.
	*/
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		/*
		Uses ICombatInterface (the I-prefixed native interface class) to call the static Execute helper function that safely
		invokes GetPlayerLevel on the target avatar. We use the I-prefixed version because Execute_GetPlayerLevel is a static
		helper function generated by Unreal for BlueprintNativeEvent functions, and it's defined in the native interface class
		(ICombatInterface), not the reflection wrapper (UCombatInterface). This Execute function handles both C++ and Blueprint
		implementations of the interface method, calling the appropriate _Implementation version.
		*/
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}
	
	// Gets the gameplay effect spec that is being executed and is the owner of this execution calculation class instance
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// Retrieves all gameplay tags that were captured from the source and target actors when the effect spec was created
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	// Creates a parameter struct that will be used when evaluating captured attribute values, allowing tag-based conditional logic
	FAggregatorEvaluateParameters EvaluationParameters;
	
	// Assigns the source tags variables we just created to the member variables of the EvaluationParameters we just created
	// so they can be used in attribute magnitude calculations
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	/*
	Calls the helper function DetermineDebuff to evaluate whether debuff effects should be applied to the target based on
	the damage types dealt by this attack. This function must be called before the main damage calculation loop, because it
	needs to check each damage type that was set via SetByCaller on the spec (e.g., Fire damage, Lightning damage) and
	determine if its corresponding debuff (e.g., Burn, Stun) should be applied based on the attacker's debuff chance and
	the target's resistance to that debuff type.

	Parameters passed:
	- ExecutionParams: Provides access to source/target ASCs and the AttemptCalculateCapturedAttributeMagnitude function
	  needed to retrieve resistance attribute values from the target
	- Spec: The gameplay effect spec containing SetByCaller damage values and debuff chance that determine which debuffs
	  to check and their application probability
	- EvaluationParameters: Contains captured source/target tags that enable tag-based conditional modifiers on resistance
	  attributes (e.g., "reduce Fire Resistance by 25% if target has Debuff.Weakened")
	- TagsToCaptureDefs: Map that associates resistance attribute tags with their capture definitions, allowing the function
	  to dynamically look up and retrieve the appropriate resistance value for each damage type's corresponding debuff

	The function iterates through all damage types set on the spec, calculates effective debuff application chance after
	accounting for target resistance, rolls random for debuff application, and applies the debuff if successful.
	*/
	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters, TagsToCaptureDefs);
	
	// Variable to hold the total damage value
	float Damage = 0.f;
	
	
	/*
	Iterates through all entries in the DamageTypesToResistances map from the gameplay tags singleton.
	This map contains key-value pairs where:
	- Pair.Key is a damage type tag (e.g., Damage.Fire, Damage.Lightning, Damage.Arcane, Damage.Physical)
	- Pair.Value is the corresponding resistance attribute tag (e.g., Attributes.Resistance.Fire)

	This approach allows the calculation to dynamically handle any number of damage types without hardcoding
	each type individually. If new damage types are added to the map in FFoxGameplayTags, they will automatically
	be processed here without requiring code changes. The loop first reduces the damage by each of the resistances
	then it accumulates damage from all damage types that were specified when the gameplay effect spec was created.
	*/
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair  : FFoxGameplayTags::Get().DamageTypesToResistances)
	{
		// Variables to store the damage type and resistance tags of the current iteration 
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		/*
		Runtime validation check that ensures the current resistance tag exists as a key in the TagsToCaptureDefs map.
		checkf is a runtime assertion macro that crashes the program if the condition is false, displaying the formatted error message.
		This is a defensive programming practice that catches configuration errors early during development:
		- If a new damage type/resistance pair is added to DamageTypesToResistances in FFoxGameplayTags
		- But the corresponding resistance attribute capture definition wasn't added to TagsToCaptureDefs in Execute_Implementation
		- This check will immediately crash with a clear error message identifying the missing tag
		Without this check, accessing a non-existent map key with operator[] would cause undefined behavior or a harder-to-debug crash.
		The 'f' in checkf means "formatted" - it allows printf-style formatting of the error message using TEXT() macro.
		`*ResistanceTag.ToString()` dereferences the FString returned by ToString() to pass it to the %s format specifier.
		*/
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());

		/*
		Retrieves the FGameplayEffectAttributeCaptureDefinition for the current resistance attribute from the TagsToCaptureDefs map
		using the operator[] lookup with ResistanceTag as the key. This map was populated earlier in Execute_Implementation and
		associates each resistance tag (e.g., Attributes.Resistance.Fire) with its corresponding capture definition
		(e.g., FireResistanceDef from DamageStatics()) that specifies:
		- Which attribute to capture (the resistance attribute property pointer from FoxDamageStatics)
		- From where to capture it (Target actor's attribute set, since resistances belong to the defender)
		- When to capture it (false = live value during execution, not snapshot at effect creation)

		The operator[] performs a map lookup using ResistanceTag as the key and returns a copy of the associated capture definition value
		for that resistance type.
		This CaptureDef will then be passed to AttemptCalculateCapturedAttributeMagnitude to retrieve the actual resistance attribute value
		from the target's attribute set, allowing us to reduce the damage by the appropriate resistance percentage before accumulating
		the final damage total.
		*/
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		/*
		Retrieves the damage magnitude for this specific damage type from the gameplay effect spec using the
		SetByCaller mechanism. SetByCaller is a powerful feature that allows abilities to dynamically specify
		numeric values at runtime when creating the gameplay effect spec, rather than requiring fixed values
		to be defined in the gameplay effect asset itself.

		How SetByCaller works:
		1. When an ability creates a gameplay effect spec, it can call SetSetByCallerMagnitude() with a tag and value
		2. The tag acts as a unique key (in this case, a damage type tag like Damage.Fire)
		3. The value is the amount of damage for that type (e.g., 50.0 for 50 fire damage)
		4. Multiple SetByCaller values can be set on the same spec with different tags
		5. Here we retrieve that stored value using GetSetByCallerMagnitude() with the damage type tag as the key

		Example: An ability could set Damage.Fire = 50.0 and Damage.Lightning = 25.0 on the spec,
		and this code would retrieve and accumulate both values across loop iterations.
		
		We pass the `Pair.Key` to `GetSetByCallerMagnitude()` to retrieve the damage value for this specific type.
		`Pair.Value` is the attribute tag that will be used to apply resistance to this damage type.
		
		We set the WarnIfNotFound parameter to false to avoid logging warnings when a damage type is not found, since we
		check each damage type and not every attack has each damage type. So it is a normal situation and we don't want to
		pollute the logs with unnecessary warnings.
		*/
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);
		
		// Variable to store the value of the current resistance attribute in this iteration of the loop
		float Resistance = 0.f;
		
		
		/*
		ExecutionParams is a const reference to FGameplayEffectCustomExecutionParameters - a struct that provides 
		context and utility functions for execution calculations. It contains the owning spec, source/target ASCs,
		and methods to retrieve captured attribute values.

		AttemptCalculateCapturedAttributeMagnitude is a helper method that:
		- Takes a capture definition (CaptureDef) specifying which attribute to retrieve and from where (Source/Target)
		- Uses evaluation parameters (tags, modifiers) to calculate the final magnitude of that attribute:
		  * The EvaluationParameters struct contains SourceTags and TargetTags that were captured when the effect spec was created
		  * These tags can influence attribute calculations through tag-based conditional modifiers defined in the attribute set or gameplay effects
		  * For example, a modifier might say "if target has Tag.Debuff.Weakened, reduce Fire Resistance by 20%"
		  * The method evaluates all such modifiers that match the current tags, aggregates them with the base attribute value,
		    and computes the final effective magnitude that should be used in this calculation
		  * This allows attributes to be dynamically modified based on active buffs, debuffs, or situational gameplay states
		- Outputs the calculated value to the variable passed by reference (Resistance)
		- Returns true if successful, false if the attribute wasn't captured or calculation failed
		*/
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		
		// Clamps the calculated resistance value to ensure it's within the valid range of 0 to 100
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		// Each point in resistance reduces the damage of its corresponding type by 1% of the total damage
		DamageTypeValue *= ( 100.f - Resistance ) / 100.f;

		// Accumulates the damage value for this damage type into the running total, allowing abilities to deal
		// multiple types of damage simultaneously (e.g., 50 fire + 25 lightning = 75 total base damage).
		Damage += DamageTypeValue;
	}
	
	// Capture BlockChance on Target, and determine if there was a successful Block
	
	// Declare and initialize a local float variable to hold the target's BlockChance attribute value, starting at 0.
	float TargetBlockChance = 0.f;
	
	// Captures the value of the target's BlockChance attribute. See the first call to this function for a more detailed comment
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	
	// Clamps the TargetBlockChance value to ensure it is never negative by taking the maximum of the captured value and 
	// 0, preventing invalid negative block chances from breaking the block calculation logic.
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	// Generates a random integer between 1 and 100 (inclusive) and compares it against TargetBlockChance to determine 
	// if the attack was blocked. If the random number is less than BlockChance, the block succeeds (e.g., 25 BlockChance 
	// means 25% chance to block).
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	
	// Access the Gameplay Effect Context associated with the Gameplay Effect using this Exec Calc, and set its blocked boolean
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	
	// Set the blocked hit boolean in the effect context to the value of bBlocked
	UFoxAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	
	// If Block, divide damage by 2.
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	// Declare and initialize a local float variable to hold the target's Armor attribute value, starting at 0.
	float TargetArmor = 0.f;
	
	// Captures the value of the target's Armor attribute. See the first call to this function for a more detailed comment
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	
	// Clamps the TargetArmor value to ensure it is never negative by taking the maximum of the captured value and 
	// 0, preventing invalid negative block chances from breaking the block calculation logic.
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);
	
	// Declare and initialize a local float variable to hold the target's ArmorPenetration attribute value, starting at 0.
	float SourceArmorPenetration = 0.f;
	
	// Captures the value of the source's ArmorPenetration attribute. See the first call to this function for a more detailed comment
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	
	// Clamps the SourceArmorPenetration value to ensure it is never negative by taking the maximum of the captured value and 
	// 0, preventing invalid negative block chances from breaking the block calculation logic.
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);
	
	/*
	Retrieves the CharacterClassInfo data asset from the game mode via a static helper function.
	This data asset contains game-wide configuration data including damage calculation coefficient curves
	that scale attributes based on character level. The SourceAvatar is passed as the world context object
	to access the current game mode instance. This gives us access to the curve tables needed for level-based
	scaling of armor penetration and effective armor calculations below.
	*/
	const UCharacterClassInfo* CharacterClassInfo = UFoxAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	/*
	Finds and retrieves the "ArmorPenetration" curve from the DamageCalculationCoefficients curve table.
	This curve table (a UCurveTable asset) stores multiple named curves, each mapping level values to coefficient values.
	FindCurve() searches for a curve by name (converted to FName) and returns a pointer to the FRealCurve data structure.
	The second parameter is an optional context string for error messages (empty string here means no custom context).
	This curve defines how armor penetration effectiveness scales with the attacker's level.
	*/
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());

	/*
	Evaluates the ArmorPenetration curve at the source character's current level to get the scaling coefficient.
	Eval() performs interpolation on the curve to return the Y-axis value (coefficient) for the given X-axis value (level).
	For example, at level 10 the curve might return 0.25, meaning each point of ArmorPenetration stat ignores 0.25% of
	target armor. This coefficient will be multiplied by SourceArmorPenetration to calculate the actual percentage of
	armor ignored in the damage calculation.
	*/
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
	
	// ArmorPenetration ignores a percentage of the Target's Armor.	SourceArmorPenetration is scaled by 
	// ArmorPenetrationCoefficient. For example, if ArmorPenetrationCoefficient is 0.25,
	// then 4 points of SourceArmorPenetration result in 1 % of
	// of TargetArmor ignored
	const float EffectiveArmor = TargetArmor * ( 100 - SourceArmorPenetration * ArmorPenetrationCoefficient ) / 100.f;
	
	/*
	Finds and retrieves the "EffectiveArmor" curve from the DamageCalculationCoefficients curve table.
	This curve table (a UCurveTable asset) stores multiple named curves, each mapping level values to coefficient values.
	FindCurve() searches for a curve by name (converted to FName) and returns a pointer to the FRealCurve data structure.
	The second parameter is an optional context string for error messages (empty string here means no custom context).
	This curve defines how EffectiveArmor scales with the attacker's level.
	*/
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	
	/*
	Evaluates the EffectiveArmor curve at the target character's current level to get the scaling coefficient.
	Eval() performs interpolation on the curve to return the Y-axis value (coefficient) for the given X-axis value (level).
	For example, at level 10 the curve might return 0.25, meaning each point of EffectiveArmor stat ignores 0.25% of
	damage. This coefficient will be multiplied by EffectiveArmor to calculate the actual percentage of
	damage ignored in the damage calculation.
	*/
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
	
	// Armor ignores a percentage of incoming Damage. EffectiveArmor is the armor of the target that is left after the 
	// Armor penetration calculation above. EffectiveArmor is scaled by EffectiveArmorCoefficient
	Damage *= ( 100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;
	
	// See comments above for very similar code. These lines simply capture the current value of the Source's Critical Hit Chance
	// attribute from the attribute set, prevent it from being negative, and store it in SourceCriticalHitChance.
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

	// See comments above for very similar code. These lines simply capture the current value of the Target's Critical Hit Resistance
	// attribute from the attribute set, prevent it from being negative, and store it in TargetCriticalHitResistance.
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	// See comments above for very similar code. These lines simply capture the current value of the Source's Critical Hit Damage
	// attribute from the attribute set, prevent it from being negative, and store it in SourceCriticalHitDamage.
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	// See comments above for very similar code. These lines get the CriticalHitResistance curve from the curve table on the 
	// CharacterClassInfo data asset and find the value on the curve using the target's level as the key.
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

	// Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	
	// This is true and a critical hit occurs if the random number generated between 1 and 100 is less than the effective
	// critical hit chance
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	
	// Set the critical hit boolean in the effect context to the value of bCriticalHit
	UFoxAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// Double damage plus a bonus (SourceCriticalHitDamage) if critical hit
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	/*
	This line uses 'Direct Initialization' (a standard C++ constructor call). 
	It is a more efficient shorthand for: 
	const FGameplayModifierEvaluatedData EvaluatedData = FGameplayModifierEvaluatedData(...);

	It creates an evaluated data structure that packages together three critical pieces of info:
	1. Which attribute to modify: (UFoxAttributeSet::GetIncomingDamageAttribute(), we used this instead before 
	   DamageStatics().ArmorProperty to modify the armor property this is the type the note refers to below)
	   Note: The engine automatically converts this FProperty pointer into an FGameplayAttribute.
	2. How to modify it: (EGameplayModOp::Additive) 
	   This tells the system to ADD the value, rather than Override or Multiply it.
	3. By how much: (Damage) 
	   The final calculated float value.

	This 'EvaluatedData' object is the "final receipt" that the Gameplay Effect system 
	reads to apply the actual numerical change to the character.
	*/
	const FGameplayModifierEvaluatedData EvaluatedData(UFoxAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);

	/* 
   Finalizes the calculation by "committing" our results to the OutExecutionOutput. 
   
   In a Custom Execution, we don't modify attributes directly. Instead, we add 
   'EvaluatedData' (our intended change) to this output object. The Gameplay 
   Effect system then collects these results and handles the actual attribute 
   updates, networking, and UI triggers after this function returns.
	*/
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
