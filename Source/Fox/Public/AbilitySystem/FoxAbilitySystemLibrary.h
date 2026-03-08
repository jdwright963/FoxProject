// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FoxAbilitySystemLibrary.generated.h"

struct FDamageEffectParams;
class UAbilityInfo;
struct FGameplayEffectContextHandle;
class AFoxHUD;
struct FWidgetControllerParams;
class USpellMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;

/**
 * Needs comment
 */
UCLASS()
class FOX_API UFoxAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	// Function to create and populate widget controller parameters required for initializing UI widget controllers
	// This is a utility function that retrieves necessary components from the game world and packages them into
	// a FWidgetControllerParams struct, which is used by various widget controllers (Overlay, Attribute Menu, Spell Menu)
	// 
	// WorldContextObject: Context object used to access the game world and retrieve the player controller
	// OutWCParams: Output parameter that will be populated with the widget controller parameters struct containing
	//              references to PlayerController, PlayerState, AbilitySystemComponent, and AttributeSet
	// OutFoxHUD: Output parameter that will be populated with a reference to the AFoxHUD instance
	// Returns: true if all required components were successfully retrieved and parameters were populated, false otherwise
	// 
	// This function is typically called before creating or initializing any widget controller to ensure all necessary
	// dependencies are available.
	// 
	// BlueprintPure: This specifier indicates that the function is a "pure" function, meaning:
	// - It does not modify game state (though it does populate output parameters)
	// - In Blueprint, it appears as a node without execution pins for cleaner blueprint graphs
	// 
	// `meta = (DefaultToSelf = "WorldContextObject")` makes the WorldContextObject input pin in BP have a default value of "Self"
	UFUNCTION(BlueprintPure, Category="FoxAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AFoxHUD*& OutFoxHUD);
	
	// Gets the Overlay Widget Controller
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	// Gets the Attribute Menu Widget Controller
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
	
	// Gets the Spell Menu Widget Controller
	UFUNCTION(BlueprintPure, Category="FoxAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);
	
	// Function that will initialize default attributes for enemies based on their class and level
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);
	
	// Function to initialize enemy gameplay abilities
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);
	
	// Function to return the UCharacterClassInfo data asset
	UFUNCTION(BlueprintCallable, Category="FoxAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
	
	// Function to retrieve the UAbilityInfo data asset from the game mode, which contains configuration data for all
	// abilities in the game including their tags, icons, materials, level requirements, and ability classes
	// This is a utility function that provides centralized access to ability information used by UI systems (such as
	// the spell menu) and gameplay systems that need to query ability metadata
	// 
	// WorldContextObject: Required context object to access the world and retrieve the game mode instance
	// Returns: Pointer to the UAbilityInfo data asset configured in AFoxGameModeBase, or nullptr if not found
	UFUNCTION(BlueprintCallable, Category="FoxAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
	
	// Function to get the value of the IsBlockedHit boolean from our custom gameplay effect context 
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	
	// Function to get the value of the IsSuccessfulDebuff boolean from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This indicates whether a debuff was successfully applied to the target actor
	// EffectContextHandle: Handle to the gameplay effect context containing the debuff success status
	// Returns: true if the debuff was successfully applied, false otherwise
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to get the damage value associated with a debuff from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This represents the amount of damage that will be applied per tick of the debuff effect
	// EffectContextHandle: Handle to the gameplay effect context containing the debuff damage value
	// Returns: The damage value that will be applied by the debuff effect on each tick
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to get the duration of a debuff effect from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This represents the total time (in seconds) that the debuff will remain active on the target
	// EffectContextHandle: Handle to the gameplay effect context containing the debuff duration value
	// Returns: The duration in seconds for which the debuff effect will be active
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to get the frequency at which a debuff effect applies damage from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This represents how often (in seconds) the debuff will apply its damage to the target
	// For example, a frequency of 1.0 means the debuff applies damage every second
	// EffectContextHandle: Handle to the gameplay effect context containing the debuff frequency value
	// Returns: The time interval in seconds between each application of debuff damage
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to get the damage type gameplay tag from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This tag identifies the type of damage being applied (e.g., Fire, Lightning, Physical, Arcane)
	// which is used to determine resistances, weaknesses, and visual effects
	// EffectContextHandle: Handle to the gameplay effect context containing the damage type tag
	// Returns: FGameplayTag representing the type of damage associated with this effect
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);
	
	// Function to get the death impulse vector from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This vector represents the physical force/impulse that should be applied to a character's ragdoll
	// upon death, typically used to create realistic death physics (e.g., flying back from an explosion)
	// EffectContextHandle: Handle to the gameplay effect context containing the death impulse vector
	// Returns: FVector representing the direction and magnitude of the impulse to apply on death
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);
	
	// Function to get the knockback force vector from our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This vector represents the physical force that should be applied to a character to push them back
	// when hit by an attack or ability, typically used to create impact feedback and positioning mechanics
	// (e.g., being pushed back by a powerful spell or melee attack)
	// EffectContextHandle: Handle to the gameplay effect context containing the knockback force vector
	// Returns: FVector representing the direction and magnitude of the knockback force to apply
	// This function is exposed to Blueprint as BlueprintPure because it only reads data and does not modify it
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to get the value of the IsBlockedHit boolean from our custom gameplay effect context. See comment above
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	// Function to set the value of the IsBlockedHit boolean from our custom gameplay effect context 
	// This function is exposed to Blueprint as BlueprintCallable because it actually modifies data
	// In UE if a parameter is passed by non const reference it is an output pin/parameter in the blueprint
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be and input pin/parameter
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	// Function to set the value of the IsCriticalHit boolean from our custom gameplay effect context. See comment above
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);
	
	// Function to set the value of the IsSuccessfulDebuff boolean in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This marks whether a debuff was successfully applied to the target actor
	// EffectContextHandle: Handle to the gameplay effect context where the debuff success status will be stored (passed by reference)
	// bInSuccessfulDebuff: Boolean value indicating whether the debuff was successfully applied
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);

	// Function to set the damage value associated with a debuff in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This sets the amount of damage that will be applied per tick of the debuff effect
	// EffectContextHandle: Handle to the gameplay effect context where the debuff damage value will be stored (passed by reference)
	// InDamage: The damage value that will be applied by the debuff effect on each tick
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDamage);

	// Function to set the duration of a debuff effect in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This sets the total time (in seconds) that the debuff will remain active on the target
	// EffectContextHandle: Handle to the gameplay effect context where the debuff duration value will be stored (passed by reference)
	// InDuration: The duration in seconds for which the debuff effect will be active
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDuration);

	// Function to set the frequency at which a debuff effect applies damage in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This sets how often (in seconds) the debuff will apply its damage to the target
	// For example, a frequency of 1.0 means the debuff applies damage every second
	// EffectContextHandle: Handle to the gameplay effect context where the debuff frequency value will be stored (passed by reference)
	// InFrequency: The time interval in seconds between each application of debuff damage
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InFrequency);
	
	// Function to set the damage type gameplay tag in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This tag identifies the type of damage being applied (e.g., Fire, Lightning, Physical, Arcane)
	// which is used to determine resistances, weaknesses, and visual effects
	// EffectContextHandle: Handle to the gameplay effect context where the damage type tag will be stored (passed by reference)
	// InDamageType: FGameplayTag representing the type of damage to associate with this effect
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);
	
	// Function to set the death impulse vector in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This vector represents the physical force/impulse that should be applied to a character's ragdoll
	// upon death, typically used to create realistic death physics (e.g., flying back from an explosion)
	// EffectContextHandle: Handle to the gameplay effect context where the death impulse vector will be stored (passed by reference)
	// InImpulse: FVector representing the direction and magnitude of the impulse to apply on death
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse);
	
	// Function to set the knockback force vector in our custom gameplay effect context
	// FFoxGameplayEffectContext from FoxAbilityTypes.h
	// This vector represents the physical force that should be applied to a character to push them back
	// when hit by an attack or ability, typically used to create impact feedback and positioning mechanics
	// (e.g., being pushed back by a powerful spell or melee attack)
	// EffectContextHandle: Handle to the gameplay effect context where the knockback force vector will be stored (passed by reference)
	// InForce: FVector representing the direction and magnitude of the knockback force to apply
	// This function is exposed to Blueprint as BlueprintCallable instead of BlueprintPure because it modifies data
	// `UPARAM(ref)` tells the engine that this parameter is supposed to be an input pin/parameter in Blueprint
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce);
	
	// Function to get all live players within a specified radius from a given origin point
	// This is a BlueprintCallable function that performs a sphere overlap query to find actors
	// WorldContextObject: Required context object to access the world (needed for world queries)
	// OutOverlappingActors: Output parameter (passed by reference) that will be populated with all actors found within the radius
	// ActorsToIgnore: Array of actors that should be excluded from the results (e.g., the instigator)
	// Radius: The radius of the sphere in which to search for actors
	// SphereOrigin: The center point (world space location) from which the sphere overlap check is performed
	// Typically used for area of effect abilities, proximity checks, or finding targets within a certain range
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);
	
	// Function to filter and sort a list of actors by distance, returning only the closest ones up to a maximum count
	// This is useful for abilities that need to target the nearest enemies within a group (e.g., chain lightning, multi-target spells)
	// The function calculates the distance from each actor to the origin point, sorts them by proximity, and returns
	// up to MaxTargets number of actors, prioritizing the closest ones
	// 
	// MaxTargets: The maximum number of closest actors to return (e.g., if MaxTargets is 3, only the 3 closest actors are returned)
	// Actors: The input array of actors to evaluate and sort by distance from the origin point
	// OutClosestTargets: Output parameter (passed by reference) that will be populated with the closest actors, sorted by distance
	// Origin: The world space position from which distances to all actors are calculated
	// 
	// Note: If the input Actors array contains fewer actors than MaxTargets, all actors will be returned and
	// the output array *** will NOT be sorted at all!!! ***
	// 
	// BlueprintCallable: This function can be called from Blueprint graphs, allowing designers to implement
	//                    proximity-based targeting logic for abilities without writing C++ code
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);
	
	// Function to check if two actors are not friends (i.e., they are enemies or neutral to each other)
	// This is typically used in gameplay scenarios to determine if an actor can target or damage another actor
	// FirstActor: The first actor whose relationship to check
	// SecondActor: The second actor whose relationship to check
	// Returns true if the actors are not friends (enemies or neutral), false if they are friends
	// 
	// BlueprintPure: This specifier indicates that the function is a "pure" function, meaning:
	// - It does not modify any state or data (no side effects)
	// - It only reads data and returns a result
	// - In Blueprint, it appears as a node without execution pins
	// - It can be called multiple times without affecting the game state
	// - It's deterministic: given the same inputs, it always produces the same output
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);
	
	// Function to apply a damage gameplay effect to a target actor using the provided damage parameters
	// This is a centralized utility function that handles the complete damage application pipeline.
	// 
	// Returns: FGameplayEffectContextHandle containing the context of the applied damage effect, which can be
	//          used to query information about the damage application (source, target, hit result, custom data)
	// 
	// BlueprintCallable: This function can be called from Blueprint graphs, allowing designers to apply
	//                    damage effects with full parameter control without writing C++ code
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);
	
	// Function to generate an array of evenly spaced rotators distributed across a specified angular spread
	// This is useful for creating symmetric spread patterns for projectiles, rays, or other directional effects
	// For example, spawning 5 projectiles with a 90-degree spread will create rotators at -45°, -22.5°, 0°, 22.5°, and 45°
	// 
	// Forward: The base forward direction vector that serves as the center of the spread pattern
	// Axis: The axis of rotation around which the spread is distributed (e.g., FVector::UpVector for horizontal spread)
	// Spread: The total angular spread in degrees across which rotators are distributed
	// NumRotators: The number of evenly spaced rotators to generate within the spread angle
	// Returns: Array of FRotator objects representing the calculated rotation directions
	// 
	// BlueprintPure: This specifier indicates that the function is a "pure" function, meaning:
	// - It does not modify any state or data (no side effects)
	// - It only performs calculations and returns a result
	// - In Blueprint, it appears as a node without execution pins 
	// - It's deterministic: given the same inputs, it always produces the same output
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	// Function to generate an array of evenly spaced direction vectors distributed across a specified angular spread
	// This is useful for creating symmetric spread patterns for projectiles, rays, or other directional effects
	// Similar to EvenlySpacedRotators but returns normalized direction vectors instead of rotators
	// For example, spawning 5 projectiles with a 90-degree spread will create direction vectors evenly distributed
	// across the spread angle, which can be directly used for setting projectile velocities or trace directions
	// 
	// Forward: The base forward direction vector that serves as the center of the spread pattern
	// Axis: The axis of rotation around which the spread is distributed (e.g., FVector::UpVector for horizontal spread)
	// Spread: The total angular spread in degrees across which vectors are distributed
	// NumVectors: The number of evenly spaced direction vectors to generate within the spread angle
	// Returns: Array of normalized FVector objects representing the calculated directional vectors
	// 
	// BlueprintPure: This specifier indicates that the function is a "pure" function, meaning:
	// - It does not modify any state or data (no side effects)
	// - It only performs calculations and returns a result
	// - In Blueprint, it appears as a node without execution pins 
	// - It's deterministic: given the same inputs, it always produces the same output
	UFUNCTION(BlueprintPure, Category = "FoxAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);
	
	// Function to retrieve the experience points reward for defeating an enemy of a specific character class and level
	// This function looks up the XP reward from the CharacterClassInfo DataAsset and scales it based on the character's level
	// using the FScalableFloat curve table associated with that character class
	// 
	// WorldContextObject: Required context object to access the world and retrieve the CharacterClassInfo DataAsset
	// CharacterClass: The class type of the character (Elementalist, Warrior, or Ranger) to determine base XP reward
	// CharacterLevel: The level of the character, used to scale the XP reward through the curve table
	// Returns: The calculated XP reward as an integer value after applying level-based scaling
	// BlueprintCallable: This specifier indicates that the function can be called from Blueprints
	UFUNCTION(BlueprintCallable, Category = "FoxAbilitySystemLibrary|CharacterClassDefaults")
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);
};
