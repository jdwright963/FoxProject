// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "FoxAttributeSet.generated.h"


/**
 * ATTRIBUTE_ACCESSORS Macro - Generates Standard Attribute Accessor Functions:
 * 
 * This is a convenience macro that expands into four essential helper functions for each gameplay attribute,
 * eliminating repetitive boilerplate code when declaring attributes in a UAttributeSet-derived class. It combines
 * four engine-provided macros from GameplayAbilities to create a complete set of attribute access patterns following
 * Unreal's Gameplay Ability System (GAS) conventions.
 * 
 * Usage: ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Health)
 * 
 * This single macro invocation generates all four functions needed to properly interact with the Health attribute
 * through the Gameplay Ability System, both for internal engine use and for gameplay code convenience.
 * 
 * @param ClassName - The name of the AttributeSet class that owns this attribute (e.g., UFoxAttributeSet). This is
 * used to properly scope the generated static getter function and ensure the attribute can be correctly identified
 * by the GAS reflection system for gameplay effect targeting and attribute queries.
 * 
 * @param PropertyName - The name of the FGameplayAttributeData member variable (e.g., Health, Mana, Strength). This
 * is used to generate function names and access the attribute's BaseValue and CurrentValue within the AttributeSet.
 * 
 * The macro expands into four function-generating sub-macros:
 * 
 * 1. GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)
 *	Expands to: static FGameplayAttribute GetPropertyNameAttribute()
 *	Example: static FGameplayAttribute UFoxAttributeSet::GetHealthAttribute()
 *	
 *	Returns an FGameplayAttribute struct that serves as a handle/identifier for this specific attribute within the
 *	GAS framework. This is used throughout GAS for targeting attributes in gameplay effects, querying attribute
 *	values by identifier, and registering attributes with the ability system. The static function allows obtaining
 *	the attribute identifier without needing an instance of the AttributeSet.
 * 
 * 2. GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)
 *	Expands to: FORCEINLINE float GetPropertyName() const
 *	Example: FORCEINLINE float GetHealth() const { return Health.GetCurrentValue(); }
 *	
 *	Returns the current value of the attribute, including all active modifier effects (buffs, debuffs, temporary
 *	changes). This is the most commonly used getter for gameplay logic that needs to read the effective attribute
 *	value. The FORCEINLINE optimization ensures no function call overhead. The const qualifier prevents modification
 *	of the AttributeSet state, making it safe to call from const contexts.
 * 
 * 3. GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)
 *	Expands to: FORCEINLINE void SetPropertyName(float NewVal)
 *	Example: FORCEINLINE void SetHealth(float NewVal) { Health.SetBaseValue(NewVal); Health.SetCurrentValue(NewVal); }
 *	
 *	Sets both the base value and current value of the attribute to NewVal, effectively replacing the attribute value
 *	and clearing any modifier calculations. This should be used sparingly in gameplay code - prefer using GameplayEffects
 *	for attribute modification to ensure proper replication, callbacks, and GAS integration. Typical use cases are
 *	direct attribute manipulation outside the normal GAS pipeline or when completely overriding an attribute value.
 * 
 * 4. GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
 *	Expands to: FORCEINLINE void InitPropertyName(float NewVal)
 *	Example: FORCEINLINE void InitHealth(float NewVal) { Health.SetBaseValue(NewVal); Health.SetCurrentValue(NewVal); }
 *	
 *	Functionally identical to the setter, but semantically distinct to indicate initialization rather than modification.
 *	This is the preferred method for setting attribute starting values during AttributeSet construction, actor spawning,
 *	or loading saved games. The separate name makes code intent clearer when reading initialization logic versus
 *	runtime attribute changes.
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * This is a convenience struct that organizes all relevant actor, component, and controller references
 * involved in a gameplay effect execution into a single structured container. It serves as a data transfer
 * object (DTO) that simplifies passing context information between gameplay effect callback functions,
 * eliminating the need to repeatedly extract source and target information from FGameplayEffectModCallbackData.
 * 
 * When a gameplay effect executes (such as damage, healing, or buff application), the Gameplay Ability System
 * provides callback data that contains context about who applied the effect (source/instigator) and who
 * received it (target). This struct extracts and caches that information in an easily accessible format,
 * organizing it into parallel source and target property groups for convenient access during attribute
 * modification calculations, damage processing, effect application logic, or any other gameplay effect responses.
 * 
 * Default Constructor: The empty default constructor FEffectProperties() {} is provided to allow default
 * initialization with all pointer members initialized to nullptr automatically by their default initializers.
 * This ensures the struct is in a safe state when created, with no dangling pointers, before being populated
 * by SetEffectProperties().
 */
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()
	
	FEffectProperties() {}
	
	/**
	 * EffectContextHandle - Gameplay Effect Execution Context:
	 * 
	 * FGameplayEffectContextHandle is a handle to the context information about how and why a gameplay effect
	 * was applied. It contains metadata about the effect's execution including the instigator (who caused the
	 * effect), the effect causer (the actor/object that directly applied it, like a projectile), hit result
	 * information if applicable, and other contextual data. This handle is used to extract source and target
	 * information during gameplay effect callbacks, enabling the system to determine who applied an effect
	 * and who received it for proper gameplay logic, damage attribution, and network replication.
	 */
	FGameplayEffectContextHandle EffectContextHandle;
	
	// Source's (the instigator) Ability System Component:
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	/**
	 * SourceAvatarActor - Source Entity's Physical Actor:
	 * 
	 * References the physical AActor in the world that represents the source entity (the instigator). This is
	 * obtained from the SourceASC's avatar actor and represents the actual gameplay actor that initiated the
	 * effect.
	 */
	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;
	
	// Points to the AController (AI or Player) that controls the source entity. Controllers handle input,
	// decision-making, and possession of pawns/characters.
	UPROPERTY()
	AController* SourceController = nullptr;

	/**
	 * SourceCharacter - Source Entity's Character:
	 * 
	 * References the ACharacter of the source entity if the source avatar actor is or is possessed by a character.
	 * ACharacter provides access to movement, animation, mesh, and character-specific functionality. This is a
	 * convenience cast from SourceAvatarActor, used when character-specific operations are needed such as playing
	 * attack animations, accessing equipped weapons, or character-based calculations. Will be nullptr if the
	 * source is not a character (e.g., a turret or environmental hazard).
	*/
	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;
	
	// TargetASC - Target Entity's Ability System Component:
	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	/**
	 * TargetAvatarActor - Target Entity's Physical Actor:
	 * 
	 * References the physical AActor in the world that represents the target entity (the receiver). This is obtained
	 * from the TargetASC's avatar actor and represents the actual gameplay actor receiving the effect.
	 */
	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	// The AController (AI or Player) that controls the target entity.
	UPROPERTY()
	AController* TargetController = nullptr;

	/**
	 * TargetCharacter - Target Entity's Character:
	 * 
	 * References the ACharacter of the target entity if the target avatar actor is or is possessed by a character.
	 * This is a convenience cast from TargetAvatarActor, providing access to character-specific functionality like
	 * playing hit reactions, ragdoll physics, character mesh for attachment, or character-based defensive calculations.
	 * Will be nullptr if the target is not a character (e.g., a destructible prop or building). UPROPERTY ensures
	 * garbage collection tracking.
	 */
	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 * TStaticFuncPtr - Template Alias for Static Function Pointers:
 * 
 * Provides a readable type alias for static function pointers compatible with Unreal's delegate system.
 * Wraps TBaseStaticDelegateInstance to create function pointer types matching any signature T.
 * 
 * Usage: TStaticFuncPtr<FGameplayAttribute()> stores a pointer to a static function returning FGameplayAttribute.
 * Used in TagsToAttributes map to store attribute getter function pointers for dynamic tag-based lookups.
 * 
 * Limitations: Only works with static functions, lambdas without captures, or global functions.
 * Cannot store member functions (use TBaseDynamicDelegate instead).
 */
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

/**
 * 
 */
UCLASS()
class FOX_API UFoxAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	
	// Constructor for this class
	UFoxAttributeSet();
	
	/**
	 * GetLifetimeReplicatedProps - Network Replication Registration Function:
	 * 
	 * This is a virtual function override from UObject that registers which properties in this class should be
	 * replicated across the network from server to clients. It's called automatically by Unreal's networking system
	 * during initialization to build a list of replicated properties for this AttributeSet.
	 * 
	 * virtual - Indicates this function overrides a base class implementation (from UObject), allowing this class
	 * to customize which properties get replicated.
	 * 
	 * void - Returns nothing; it modifies the OutLifetimeProps array parameter instead.
	 * 
	 * TArray<FLifetimeProperty>& OutLifetimeProps - A reference to an array that will be populated with replication
	 * metadata for each property that should replicate. Inside the implementation, we use DOREPLIFETIME or similar
	 * macros to add our replicated properties (like Health) to this array, specifying replication conditions such as
	 * whether to replicate to owner only, to all clients, replication condition checks, etc.
	 * 
	 * const - This function doesn't modify the object's state, only populates the output parameter.
	 * 
	 * override - C++11 keyword ensuring this function actually overrides a base class virtual function, providing
	 * compile-time safety by generating an error if the signature doesn't match any base class virtual function.
	 * 
	 * The typical implementation uses DOREPLIFETIME(UFoxAttributeSet, Health) or DOREPLIFETIME_CONDITION_NOTIFY macros
	 * to register each UPROPERTY marked with ReplicatedUsing, establishing the server-to-client replication pipeline
	 * for all gameplay attributes in this AttributeSet.
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Fires before any attribute changes. Epic recommends we only use this function to do clamping and no other gameplay
	// logic
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	// Needs comment explaining input parameters
	// Executed after a gameplay effect changes an attribute
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	/**
	 * Notification callback fired after any attribute value changes in this AttributeSet,
	 * regardless of the source (gameplay effects, direct modification, replication, or initialization).
	 * 
	 * Unlike PostGameplayEffectExecute which only fires for gameplay effect-driven changes, this executes for
	 * ALL attribute modifications, making it ideal for generic responses like UI updates, event broadcasting,
	 * or triggering effects that should occur whenever an attribute changes.
	 * 
	 * Execution Order: PreAttributeChange → Attribute Modified → PostAttributeChange → PostGameplayEffectExecute (if from GE)
	 * 
	 * @param Attribute - Identifies which attribute changed, allowing attribute-specific response logic
	 * @param OldValue - The attribute's value before the change, useful for calculating deltas
	 * @param NewValue - The attribute's value after the change and any clamping from PreAttributeChange
	 */
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	// Map contains key (Gameplay Tag) value (function pointers with FGameplayAttribute() signature) pairs.
	// FGameplayAttribute is the type returned and it has zero params
	/**
	 * Function Pointer Type Definition for Attribute Getter Storage:
	 * 
	 * This was moved to a using statement above the class, and the alias is used below:
	 * TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr - This declares a type
	 * alias for a raw C++ function pointer that can store references to static functions or lambdas that match the
	 * signature: FGameplayAttribute(). This is the underlying storage mechanism used by Unreal's delegate system.
	 * 
	 * TBaseStaticDelegateInstance - A template class from Unreal's delegate system that represents a delegate instance
	 * bound to a static function or lambda (as opposed to member functions which use different delegate types). It's
	 * the base implementation that stores and invokes static function pointers within the delegate framework.
	 * 
	 * <FGameplayAttribute()> - The first template parameter specifying the function signature: a function that takes
	 * no parameters (empty parentheses) and returns FGameplayAttribute. This matches our attribute getter functions
	 * like GetHealthAttribute() which return the FGameplayAttribute metadata for a specific attribute.
	 * 
	 * FDefaultDelegateUserPolicy - The second template parameter defining the delegate's behavior policy, such as
	 * thread-safety guarantees, memory management, and invocation rules. The default policy provides standard
	 * single-threaded delegate behavior suitable for most gameplay code.
	 * 
	 * ::FFuncPtr - A typedef within TBaseStaticDelegateInstance that represents the actual raw C++ function pointer
	 * type. This is the underlying primitive pointer (like FGameplayAttribute(*)()) that stores the memory address
	 * of the function to be called.
	 * 
	 * FunctionPointer - The variable name being declared with this type. In practice, this can store a pointer to
	 * any static function or lambda that matches the FGameplayAttribute() signature, such as the attribute getter
	 * functions generated by ATTRIBUTE_ACCESSORS macro (e.g., UFoxAttributeSet::GetHealthAttribute).
	 * 
	 * Usage Context: While this specific variable isn't directly used in the visible code, this type demonstrates
	 * the underlying mechanism of how the TagsToAttributes map stores function references. The map's values
	 * (FAttributeSignature delegates) internally use similar function pointer storage to bind and execute the
	 * attribute getter lambdas, enabling dynamic attribute lookups by GameplayTag at runtime for UI updates
	 * and attribute system queries.
	 * 
	 * Note: The double semicolon (;;) at the end appears to be a typo and should be a single semicolon.
	 */
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
	
	// This is the same as the above map except it uses the raw C++ syntax for a function pointer. It maps gameplay tags
	// to function pointers. DO NOT DELETE!
	//TMap<FGameplayTag, FGameplayAttribute(*)()> TagsToAttributes;
	
	/*
	 * Primary Attributes
	 */
	
	/**
	 * Strength Attribute Declaration with Network Replication and Blueprint Access:
	 * 
	 * UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes") - This macro
	 * marks the Strength property for Unreal's reflection system, enabling Blueprint access and network replication
	 * 
	 * - BlueprintReadOnly: Exposes this attribute to Blueprint graphs as a read-only property. Blueprint scripts can
	 *   query the Strength value but cannot directly modify it (modifications should go through the Gameplay Ability
	 *   System to ensure proper attribute modification pipeline, callbacks, and network replication).
	 * 
	 * - ReplicatedUsing = OnRep_Strength: Enables automatic network replication from server to clients. When the
	 *   server modifies Strength's value, the engine automatically synchronizes it to all clients and invokes the
	 *   OnRep_Strength callback function on clients, allowing client-side response logic like UI updates or visual
	 *   effects without additional network messages.
	 * 
	 * FGameplayAttributeData Strength - Declares the actual attribute data member using Gameplay Ability System's
	 * attribute data structure. FGameplayAttributeData is a specialized struct that contains:
	 * - BaseValue: The permanent base value of the attribute before any temporary modifiers
	 * - CurrentValue: The final computed value including all active gameplay effect modifiers
	 * - Internal tracking for attribute modifiers, aggregators, and modifier stacks
	 * This type integrates seamlessly with GameplayEffects, allowing modifiers to stack, expire, and recalculate
	 * automatically while maintaining the distinction between base stats and temporary buffs/debuffs.
	 * 
	 * ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Strength) - This macro invocation expands into four helper functions
	 * that provide standardized access patterns for this attribute, eliminating boilerplate code:
	 * 
	 * 1. static FGameplayAttribute GetStrengthAttribute() - Returns the FGameplayAttribute metadata structure
	 *    identifying this attribute for gameplay effect targeting, attribute queries, and runtime attribute
	 *    lookups by the ability system.
	 * 
	 * 2. float GetStrength() const - Inline getter returning the current value of Strength (Strength.GetCurrentValue()),
	 *    providing convenient read access to the final computed attribute value including all active modifiers.
	 * 
	 * 3. void SetStrength(float NewVal) - Inline setter that assigns NewVal to Strength's base and current values
	 *    (Strength.SetBaseValue(NewVal) and Strength.SetCurrentValue(NewVal)), typically used during initialization
	 *    or when completely overriding the attribute outside the normal gameplay effect modification pipeline.
	 * 
	 * 4. void InitStrength(float NewVal) - Inline initializer that sets both base and current values to NewVal,
	 *    functionally identical to SetStrength but semantically distinct to indicate initial attribute setup rather
	 *    than runtime modification, commonly called during actor construction or attribute set initialization.
	 * 
	 * Network Replication Flow:
	 * 1. Server modifies Strength through a GameplayEffect execution
	 * 2. PostGameplayEffectExecute runs on server, updating the attribute value
	 * 3. Unreal's replication system detects the change and sends the new value to clients
	 * 4. OnRep_Strength callback executes on each client, enabling client-side UI updates or effects
	 * 5. GAMEPLAYATTRIBUTE_REPNOTIFY macro in OnRep_Strength ensures proper GAS internal state synchronization
	 * 
	 * This three-line pattern (UPROPERTY + FGameplayAttributeData + ATTRIBUTE_ACCESSORS) is the standard declaration
	 * for every gameplay attribute in a GAS AttributeSet, providing Blueprint visibility, network replication,
	 * and convenient accessor functions with minimal code repetition.
	*/
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Strength);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Intelligence);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Resilience);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Vigor);
	
	/*
	 * Secondary Attributes
	 */
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Armor);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, ArmorPenetration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, BlockChance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, CriticalHitChance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, CriticalHitDamage);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResistance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, CriticalHitResistance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, HealthRegeneration);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, ManaRegeneration);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, MaxMana);
	
	/*
	 * Resistance Attributes
	 */
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireResistance, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, FireResistance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningResistance, Category = "Resistance Attributes")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, LightningResistance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArcaneResistance, Category = "Resistance Attributes")
	FGameplayAttributeData ArcaneResistance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, ArcaneResistance);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalResistance, Category = "Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, PhysicalResistance);
	
	/*
	 * Vital Attributes
	 */
	
	/**
	 * Health Attribute with Network Replication Setup:
	 * 
	 * UPROPERTY(ReplicatedUsing = OnRep_Health) - Marks this property for network replication from server to clients.
	 * When the server changes this value, it automatically replicates to clients and triggers the OnRep_Health callback.
	 * 
	 * FGameplayAttributeData Health - The actual attribute data structure from Gameplay Ability System (GAS).
	 * FGameplayAttributeData contains BaseValue and CurrentValue, handles attribute modifiers, and provides
	 * standardized attribute management within the GAS framework.
	 * 
	 * UFUNCTION() - Required macro for the replication callback function to be recognized by Unreal's reflection system.
	 * Without this, the RepNotify function won't be properly registered.
	 * 
	 * void OnRep_Health(const FGameplayAttributeData& OldHealth) - The RepNotify callback function that executes
	 * on clients when Health replicates. Takes the previous value as parameter, allowing you to compare old vs new
	 * values and perform client-side logic like updating UI, playing effects, or triggering animations.
	 * This function could be defined to accept no parameter as well.
	 */
	/**
	 * This comment comes from the engine code:
	 * This defines a set of helper functions for accessing and initializing attributes, to avoid having to manually write these functions.
	 * It would create the following functions, for attribute Health
	 *
	 *	static FGameplayAttribute UMyHealthSet::GetHealthAttribute();
	 *	FORCEINLINE float UMyHealthSet::GetHealth() const;
	 *	FORCEINLINE void UMyHealthSet::SetHealth(float NewVal);
	 *	FORCEINLINE void UMyHealthSet::InitHealth(float NewVal);
	 *
	 * To use this in your game you can define something like this, and then add game-specific functions as necessary:
	 * 
	 *	#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	 *	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	 *	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	 *	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	 *	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
	 * 
	 *	ATTRIBUTE_ACCESSORS(UMyHealthSet, Health)
	 */
	// Steps to add attributes to GAS:
	// Step 1
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, Mana);
	
	/* 
	 *  Meta Attributes
	 */
	
	// Meta attribute for damage. Meta attributes are not replicated. They exist only on the server. They are what is 
	// immediately effected by gameplay effects, so that complex calculations can be made before the result is applied to 
	// the actual attribute
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, IncomingDamage);
	
	// Meta attribute for XP rewards
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(UFoxAttributeSet, IncomingXP);
	
	// Step 2 to adding attributes
	/**
	 * OnRep_Health - Network Replication Notification Callback for Health Attribute:
	 * 
	 * This is a RepNotify callback function that executes on clients when the Health attribute value
	 * replicates from the server. It's automatically invoked by Unreal's networking system after the
	 * new Health value has been received and applied on the client, allowing for client-side response
	 * logic such as UI updates, visual effects, audio cues, or animations based on health changes.
	 * 
	 * UFUNCTION() - Required reflection macro that registers this function with Unreal's property system,
	 * enabling it to be used as a RepNotify callback. Without this macro, the function cannot be properly
	 * bound to the replication system and won't be called when the associated property replicates. This
	 * macro has no specifiers here because RepNotify functions are automatically recognized by the engine
	 * through the ReplicatedUsing declaration in the UPROPERTY macro of the Health attribute.
	 * 
	 * void - Return type indicating this function performs side effects (like updating UI or spawning
	 * effects) but doesn't return any value. RepNotify callbacks always have void return type as they're
	 * invoked by the engine's replication system which doesn't expect or handle return values.
	 * 
	 * OnRep_Health - Function name following Unreal's naming convention for RepNotify callbacks: OnRep_
	 * prefix followed by the property name. This naming pattern is a convention that makes it immediately
	 * clear this function is a replication callback for the Health property. The name must match exactly
	 * what's specified in the ReplicatedUsing parameter of the Health property's UPROPERTY macro.
	 * 
	 * const FGameplayAttributeData& OldHealth - Parameter providing the previous value of the Health
	 * attribute before the replication occurred, passed by const reference for efficiency (avoiding copy)
	 * and safety (preventing modification). This allows the function to compare the old value with the
	 * new replicated value (accessible via GetHealth()) to determine the magnitude and direction of change,
	 * enabling contextual responses like playing different effects for small damage versus large damage,
	 * or distinguishing between health gain and health loss. The parameter is optional - RepNotify functions
	 * can also be declared with no parameters if the old value isn't needed for the callback logic.
	 * 
	 * const - Function-level const qualifier indicating this function doesn't modify any member variables
	 * of the UFoxAttributeSet class. This is appropriate for RepNotify callbacks that only read attribute
	 * values and trigger external side effects (like UI updates or effect spawns) without changing the
	 * attribute set's state. The const qualifier provides compile-time safety, preventing accidental
	 * modification of attribute values within the callback, which should only be changed through the
	 * proper Gameplay Ability System modification pipeline to ensure network consistency.
	 * 
	 * Typical Implementation Pattern:
	 * The implementation of this function (in the .cpp file) should call GAMEPLAYATTRIBUTE_REPNOTIFY macro
	 * to ensure proper synchronization of the attribute's internal state with the GAS prediction system:
	 * 
	 *     GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Health, OldHealth);
	 * 
	 * This macro handles critical internal bookkeeping for the Gameplay Ability System's client prediction
	 * and reconciliation logic. After the macro, custom client-side logic can be added such as updating
	 * health bar widgets, playing damage/healing effects, or triggering low-health warnings.
	 * 
	 * Network Flow:
	 * 1. Server modifies Health through PostGameplayEffectExecute or direct attribute change
	 * 2. Unreal's replication system detects the change (due to ReplicatedUsing in Health's UPROPERTY)
	 * 3. New Health value is sent to all connected clients over the network
	 * 4. Each client receives the new value and updates their local Health attribute
	 * 5. OnRep_Health is automatically called on each client with the old value as parameter
	 * 6. Client executes GAMEPLAYATTRIBUTE_REPNOTIFY and any custom response logic (UI, effects, etc.)
	 */
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
	
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
	
	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
	
	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;
	
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
	
	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;
	
	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;
	
	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;
	
	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;
	
	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;
	
	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;
	
	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;
	
	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;

	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;

	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;

	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;
	
private:
	
	
	/**
	 * HandleIncomingDamage - Processes Incoming Damage Meta Attribute and Applies Final Damage to Health:
	 * 
	 * This function is called from PostGameplayEffectExecute after a gameplay effect has modified the IncomingDamage
	 * meta attribute, serving as the central damage processing pipeline that converts the meta attribute value into
	 * actual health reduction. It performs all final damage processing including reading combat calculation results
	 * from the effect context (critical hit boolean flags, block status boolean, damage type tags), spawning
	 * visual feedback through floating damage numbers, and ultimately applies the final damage to the target's Health
	 * attribute. This function also handles death detection and triggers appropriate death callbacks when health
	 * reaches zero, as well as hit react animations when damage is not fatal.
	 * 
	 * The IncomingDamage meta attribute is used as an intermediate calculation buffer that allows complex damage
	 * formulas to be computed in gameplay effects before the final result is applied to health. This separation
	 * enables modular damage calculation through gameplay effects while keeping all final processing, validation,
	 * and side effects (like floating text, death handling, XP rewards) centralized in this function.
	 * 
	 * @param Props - const FEffectProperties& - A const reference to the effect properties structure populated by
	 * SetEffectProperties, containing all source (attacker) and target (victim) actor, component, and controller
	 * references needed for damage processing. This provides access to:
	 * - TargetASC: Target's ability system component for reading/modifying Health and accessing combat attributes
	 * - SourceASC: Source's ability system component for potential damage reflection or counter-attack mechanics
	 * - TargetCharacter: Target character for death handling, animation triggers, and physics responses
	 * - SourceCharacter: Source character for XP reward attribution and kill credit assignment
	 * - EffectContextHandle: Context containing critical hit flags, block status, and damage type metadata
	 * Passed as const reference for efficiency since the function only reads from the struct without modification.
	 */
	void HandleIncomingDamage(const FEffectProperties& Props);

	/**
	 * HandleIncomingXP - Processes Incoming Experience Points Meta Attribute and Awards XP to Source Character:
	 * 
	 * This function is called from PostGameplayEffectExecute after a gameplay effect has modified the IncomingXP
	 * meta attribute, serving as the central experience point distribution pipeline that awards XP to the character
	 * responsible for the kill or event. It reads the accumulated XP value from the meta attribute and dispatches
	 * a gameplay event to the source character's ability system component, allowing passive abilities (like
	 * GA_ListenForEvent) to respond to XP gain events by updating character level, triggering level-up rewards,
	 * broadcasting UI notifications, or applying stat bonuses.
	 * 
	 * The IncomingXP meta attribute acts as a temporary buffer that accumulates experience point rewards from
	 * multiple sources (kills, quest completion, environmental interactions) before being processed and awarded
	 * to the appropriate character. This meta attribute pattern keeps XP calculation logic separate from XP
	 * distribution mechanics, allowing flexible XP formulas in gameplay effects while centralizing the actual
	 * character progression updates in listening abilities.
	 * 
	 * This function is typically triggered automatically after a successful kill when HandleIncomingDamage detects
	 * target death and calls SendXPEvent, which applies a gameplay effect that modifies IncomingXP, causing this
	 * function to execute through the PostGameplayEffectExecute callback chain.
	 * 
	* @param Props - const FEffectProperties& - A const reference to the effect properties structure populated by
	 * SetEffectProperties, containing:
	 * source (XP recipient) and target (XP source) actor, component, and controller references. This provides:
	 * - SourceASC: Source character's ability system component for sending the XP reward gameplay event
	 * - SourceCharacter: Source character receiving the XP for level tracking and progression
	 * - TargetCharacter: Target character providing the XP for determining reward amounts based on level/class
	 * - EffectContextHandle: Context information about the XP award source (kill, quest, interaction, etc.)
	 * Passed as const reference for efficiency since the function only reads from the struct without modification.
	 */
	void HandleIncomingXP(const FEffectProperties& Props);
	
	
	/**
	 * Applies Debuff Effects from Gameplay Effect Execution to Target Character
	 * 
	 * @param Props - const FEffectProperties& - A const reference to the effect properties structure populated
	 * by SetEffectProperties, containing all source (debuff applicator) and target (debuff recipient) actor,
	 * component, and controller references needed for debuff processing. This provides access to:
	 * - TargetASC: Target's ability system component for applying the debuff gameplay effect and tracking active debuffs
	 * - SourceASC: Source's ability system component for reading debuff power scaling attributes or ability levels
	 * - TargetCharacter: Target character for visual debuff indicators, animations, or particle effects
	 * - EffectContextHandle: Context containing debuff type, damage values, duration, frequency, and damage type tags
	 * Passed as const reference for efficiency since the function only reads from the struct to extract debuff
	 * application parameters without modifying the source data.
	 */
	void Debuff(const FEffectProperties& Props);

	/**
	 * SetEffectProperties - Populates Effect Property Data from Gameplay Effect Callback:
	 * 
	 * This helper function extracts and organizes all relevant actor, component, and controller information
	 * from a gameplay effect execution into a structured FEffectProperties for easy access. It's typically
	 * called at the beginning of PostGameplayEffectExecute to set up both source (instigator) and target
	 * (receiver) information needed for attribute modification logic, damage calculations, or other
	 * gameplay effect responses.
	 * 
	 * @param Data - const FGameplayEffectModCallbackData& - The callback data structure provided by the
	 * Gameplay Ability System when a gameplay effect executes and modifies an attribute. Contains the
	 * FGameplayEffectContextHandle with source/target information, the FGameplayEffectSpec with effect
	 * configuration, and the EvaluatedData with the actual attribute being modified and its magnitude.
	 * This parameter is const reference since we only read from it to populate Props.
	 * 
	 * @param Props - FEffectProperties& - A reference to the FEffectProperties struct that will be populated
	 * with extracted information. This struct will be filled with the source AbilitySystemComponent, avatar
	 * actor, controller, and character, as well as the corresponding target versions of these objects. Passed
	 * by non-const reference so the function can modify it and populate all member variables. After this
	 * function executes, Props will contain all the context needed to implement gameplay logic based on who
	 * applied the effect and who received it.
	*/
	// Function to set the values in the FEffectProperties struct
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
	
	/**
	 * This helper function spawns floating text widgets above the target actor to provide visual feedback
	 * for damage dealt, blocked hits, or critical strikes. It's typically called from PostGameplayEffectExecute
	 * after damage calculations are complete, creating player-visible UI elements that communicate combat
	 * effectiveness and help players understand the impact of their actions or incoming threats.
	 * 
	 * @param Props - const FEffectProperties& - A const reference to the effect properties structure containing
	 * both source and target actor/component references. This provides access to the target actor's location for
	 * spawning the floating text widget and potentially the source actor for color coding or directional offsets.
	 * The struct includes TargetAvatarActor for positioning the text, SourceCharacter for contextual styling, and
	 * other gameplay context that may influence text appearance or behavior. Passed as const reference for
	 * efficiency and safety since the function only reads positioning and context data without modification.
	 * 
	 * @param Damage - float - The magnitude of damage to display in the floating text. This is typically the final
	 * calculated damage value after all modifiers, resistances, armor calculations, and combat mechanics have been
	 * applied. The value determines the numeric text shown to the player, potentially influencing text size, color
	 * intensity, or animation based on damage severity. For example, higher damage values might spawn larger text
	 * with more dramatic animations to convey impact.
	 * 
	 * @param bBlockedHit - bool - Boolean flag indicating whether the hit was blocked by the target's defense
	 * mechanics (such as shield block or parry). When true, the floating text typically displays "BLOCKED" or
	 * shows reduced/negated damage with distinct visual styling (often blue or white text) to communicate that
	 * the target's defensive action was successful. This provides important combat feedback about the effectiveness
	 * of blocking mechanics and helps players understand why damage was mitigated.
	 * 
	 * @param bCriticalHit - bool - Boolean flag indicating whether the hit was a critical strike that dealt
	 * amplified damage. When true, the floating text usually appears with special styling such as larger font size,
	 * bright colors (often red or yellow), exclamation marks, or dramatic animations to emphasize the significance
	 * of landing a critical hit. This visual feedback rewards players for successful critical strikes and makes
	 * high-impact moments more satisfying and noticeable during fast-paced combat.
	*/
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;
	
	/**
	 * Function that sends an experience point reward event to the source character (the killer) after a fatal hit.
	 * 
	 * This function call triggers the XP distribution system by sending a gameplay event with the
	 * Attributes_Meta_IncomingXP tag to the source character's ability system component. The function
	 * calculates the appropriate XP reward based on the target's character class and level, packages
	 * this information into a FGameplayEventData payload, and sends it to any abilities on the
	 * source character that are listening for XP gain events (such as the passive ability GA_ListenForEvent).
	 * 
	 * @param Props - const FEffectProperties& - A const reference to the effect properties structure containing
	 * both source (killer) and target (defeated enemy) actor, component, and controller references. This provides
	 * access to the source character's ability system component for sending the XP reward event, the target
	 * character for querying class and level information to calculate appropriate XP amounts, and all gameplay
	 * context needed to properly attribute the kill and distribute experience points.
	*/
	void SendXPEvent(const FEffectProperties& Props);
	
	
	/**
	 * Boolean flag that indicates whether the character's health should be fully restored to its maximum value
	 * during the next attribute update cycle. This flag is typically set to true when a character levels up,
	 * triggering the health restoration logic in PostAttributeChange or PostGameplayEffectExecute callbacks.
	 * 
	 * Default value of false ensures health restoration only occurs when explicitly triggered by level-up
	 * mechanics, not during normal MaxHealth changes from equipment, buffs, or other attribute modifications.
	 */
	bool bTopOffHealth = false;

	/**
	 * Boolean flag that indicates whether the character's mana should be fully restored to its maximum value
	 * during the next attribute update cycle. This flag is typically set to true when a character levels up,
	 * triggering the mana restoration logic in PostAttributeChange or PostGameplayEffectExecute callbacks.
	 * 
	 * Default value of false ensures mana restoration only occurs when explicitly triggered by level-up
	 * mechanics, not during normal MaxMana changes from equipment, buffs, or other attribute modifications.
	 */
	bool bTopOffMana = false;
};
