// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;
/*
 * FFoxAbilityInfo is a data structure that encapsulates all the information needed to describe and display a gameplay 
 * ability in the UI and game systems. It stores the ability's unique identifier (AbilityTag), its associated input 
 * binding (InputTag), cooldown information (CooldownTag), and visual assets (Icon and BackgroundMaterial) used for 
 * UI representation.
 *
 * USTRUCT(BlueprintType): This macro and specifier combination makes the struct available for use in Blueprint:
 * - USTRUCT: Marks this as an Unreal Engine reflected struct, enabling it to work with Unreal's property system, 
 *   serialization, networking, and garbage collection
 * - BlueprintType: Allows this struct to be used as a variable type in Blueprint graphs. Without this specifier, 
 *   the struct would only be usable in C++ and wouldn't appear in Blueprint's variable type dropdown menus or be 
 *   passable as function parameters in Blueprint
 *
 * This struct is typically used as an element in the TArray<FFoxAbilityInfo> AbilityInformation array within the 
 * UAbilityInfo data asset, allowing designers to configure multiple abilities in a single data asset.
 */
USTRUCT(BlueprintType)
struct FFoxAbilityInfo
{
	GENERATED_BODY()
	
	// Tag that uniquely identifies this ability (e.g., "Abilities.Fire", "Abilities.Lightning")
	// Initialized to an empty FGameplayTag. Most of the variables in this struct have their values set in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();
	
	// Tag that identifies which input action activates this ability (e.g., "InputTag.LMB", "InputTag.1")
	// NOT EditDefaultsOnly: This is set at runtime in code, not in the editor, because input bindings are determined
	// when abilities are granted to characters and can vary per character configuration
	// Initialized to an empty FGameplayTag
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();
	
	// Tag that represents the current status or state of this ability (e.g., "Abilities.Status.Locked", 
	// "Abilities.Status.Unlocked", "Abilities.Status.Equipped") 
	// NOT EditDefaultsOnly: This is set at runtime in code, not in the editor, because ability status changes dynamically
	// based on player progression, ability unlocking systems, and equipment state
	// Initialized to an empty FGameplayTag
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();
	
	// Tag that identifies the cooldown effect for this ability (e.g., "Cooldown.Fire", "Cooldown.Lightning")
	// Used by systems like WaitCooldownChange to monitor when this ability is on cooldown
	// Initialized to an empty FGameplayTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();
	
	// Tag that categorizes the type of this ability (e.g., "Abilities.Type.Offensive", "Abilities.Type.Passive")
	// Used by the spell menu and equip systems to determine which input slots this ability can be assigned to.
	// Offensive abilities can be equipped to active input slots (LMB, RMB, 1-4), while passive abilities are
	// automatically active and don't require input binding. It is initialized to an empty FGameplayTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();
	
	// Pointer to a 2D texture asset representing the ability's icon for UI display
	// TObjectPtr: Unreal Engine 5's smart pointer type for UObject references (provides better performance and debugging)
	// Initialized to nullptr (no icon assigned by default)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;
	
	// Pointer to a material interface asset used as the background for the ability icon in UI
	// UMaterialInterface: Base class for materials, allowing both UMaterial and UMaterialInstance to be assigned
	// Initialized to nullptr (no background material assigned by default)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;
	
	// The minimum character level required to unlock this ability. This value is used by progression systems to
	// determine when an ability becomes available to the player. For example, a LevelRequirement of 5 means the
	// player must reach level 5 before this ability can be unlocked or equipped.
	// Initialized to 1 (ability available at level 1 by default). This is usually set to a different value in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;

	// Reference to the actual UGameplayAbility class that implements this ability's gameplay logic. This is the
	// blueprint or C++ class that defines what happens when the ability is activated, including any effects,
	// animations, costs, and cooldowns. This connects the UI/data representation (FFoxAbilityInfo) with the
	// functional implementation (UGameplayAbility subclass).
	// TSubclassOf: Template type that restricts the selection to classes derived from UGameplayAbility
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

/**
 * 
 */
UCLASS()
class FOX_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	
	// Array that stores all the instances of FFoxAbilityInfo for this data asset. This is the main container that holds
	// the configuration data for all abilities in the game. Designers populate this array in the editor by adding
	// FFoxAbilityInfo entries, each representing a different ability with its associated tags, icons, and materials.
	//
	// Usage: Game systems query this array using FindAbilityInfoForTag() to retrieve UI and configuration data for
	// specific abilities by their AbilityTag. For example, the ability widget overlay uses this to display the
	// correct icon and background material for each ability slot.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TArray<FFoxAbilityInfo> AbilityInformation;

	// Searches the AbilityInformation array for an entry matching the specified AbilityTag and returns it. This is
	// the primary lookup function used by game systems to retrieve ability display data and configuration.
	//
	// Parameters:
	// - AbilityTag: The gameplay tag that uniquely identifies the ability to search for (e.g., "Abilities.Fire",
	//   "Abilities.Lightning").
	// - bLogNotFound: Optional boolean parameter (defaults to false) that controls whether a warning message is
	//   logged if no matching ability is found. Set to true when you expect the ability to exist and want to be
	//   notified if it's missing, false when the absence of the ability is acceptable
	FFoxAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
};
