// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/**
 * Character Class Enumeration
 * 
 * This is a SCOPED ENUM (also called an "enum class") that defines the different character classes in the game.
 * 
 * What is a Scoped Enum (enum class)?
 * - A scoped enum is a type-safe enumeration introduced in C++11 that uses the 'class' keyword
 * - The 'class' keyword here does NOT mean it's a class - it creates a scoped enumeration type
 * - Scoped enums prevent name collisions by requiring the enum name as a prefix (e.g., ECharacterClass::Warrior)
 * - Values cannot be implicitly converted to integers, providing better type safety
 * - Contrast with unscoped enums (enum without 'class') where values are in the enclosing scope
 * 
 * UENUM(BlueprintType):
 * - UENUM is an Unreal macro that registers this enum with Unreal's reflection system
 * - BlueprintType specifier makes this enum accessible in Blueprints
 * 
 * : uint8 (Underlying Type):
 * - Specifies the underlying integer type used to store enum values
 * - uint8 is an unsigned 8-bit integer (0-255), taking only 1 byte of memory
 * - This is memory-efficient since we only have 3 character classes
 * - Without explicit specification, C++ defaults to 'int' (typically 4 bytes)
 * - Unreal recommends uint8 for UENUM types for memory efficiency and Blueprint compatibility
 * 
 * Enum Values:
 * - Elementalist = 0 (implicitly)
 * - Warrior = 1 (implicitly)
 * - Ranger = 2 (implicitly)
 */
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Elementalist,
	Warrior,
	Ranger
};

/**
 * Character Class Default Information Structure
 * 
 * This struct contains the default configuration data for each character class.
 * It is used to store class-specific gameplay effects and attributes that differ
 * between character classes (Elementalist, Warrior, Ranger).
 * 
 * USTRUCT(BlueprintType):
 * - USTRUCT is an Unreal macro that registers this struct with Unreal's reflection system
 * - BlueprintType makes this struct usable in Blueprints for visual scripting
 * 
 * GENERATED_BODY():
 * - Required Unreal macro that generates necessary boilerplate code for the struct
 * - Provides reflection data, serialization support, and other Unreal features
 * 
 * Usage:
 * - Instances of this struct are stored in the CharacterClassInformation TMap
 * - Each ECharacterClass enum value maps to one FCharacterClassDefaultInfo instance
 * - This allows different classes to have different primary attributes while sharing
 *   common attributes (secondary, vital) defined directly in UCharacterClassInfo
 */
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()
	
	// Gameplay effect to apply primary attributes to enemy characters. This is differenct for each class
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;
	
	// Array of Gameplay Abilities that characters should start the game with if their character class maps to this 
	// instance of this struct in the CharacterClassInformation map. Each character class is mapped to an instance of
	// this struct
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	/**
	 * Experience Points Reward for defeating this character class
	 * 
	 * FScalableFloat is a special Unreal structure that allows a float value to scale based on a curve table. It has a
	 * base float value with an optional curve table for level-based scaling. Different character classes can have 
	 * different XP rewards (e.g., bosses give more XP than regular enemies)
	 * 
	 * Default Initialization (= FScalableFloat()):
	 * - Initializes the variable with a default-constructed FScalableFloat
	 * - Default FScalableFloat has a value of 0.0f with no curve table assigned
	 * - The actual value and curve table should be set in the DataAsset editor for each character class
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
	FScalableFloat XPReward = FScalableFloat();
};

/**
 * 
 */
UCLASS()
class FOX_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	// Map to store instances of the FCharacterClassDefaultInfo struct. Maps keys (ECharacterClass) to values 
	// (FCharacterClassDefaultInfo) the contents of this map are added in the editor
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
	
	// Gameplay effect to apply Secondary attributes to enemy characters. This is shared by all classes. Placing this in
	// the FCharacterClassDefaultInfo struct will make it specific to each class
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;
	
	// Gameplay effect to apply Vital attributes to enemy characters. This is shared by all classes
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;
	
	// Array that contains all of the abilities that a character should have. This is set in the data asset in the editor
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;
	
	// Variable to store the curve table for damage calculation coefficients. This is shared by all classes.
	// The value is set in the blueprint for this data asset
	UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults|Damage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficients;
	
	// Function to retrieve the default information for a specific character class
	FCharacterClassDefaultInfo GetClassDefaultInfo(ECharacterClass CharacterClass);
};
