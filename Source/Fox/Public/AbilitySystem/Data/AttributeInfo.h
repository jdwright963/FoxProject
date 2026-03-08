// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AttributeInfo.generated.h"


// Struct to hold display and identification information for a gameplay attribute.
// This is used in the Gameplay Ability System to store metadata about attributes
// like health, mana, strength, etc. that can be shown in UI and referenced by tag.
USTRUCT(BlueprintType)
struct FFoxAttributeInfo
{
	GENERATED_BODY()
	
	// Unique identifier for this attribute using Unreal's Gameplay Tag system.
	// This tag is used to look up and match specific attributes (e.g., "Attributes.Vital.Health").
	// EditDefaultsOnly: Can only be set in the data asset, not at runtime.
	// BlueprintReadOnly: Blueprints can read but not modify this value.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();
	
	// The human-readable display name of the attribute shown in UI (e.g., "Health", "Mana").
	// Uses FText for localization support so the name can be translated to different languages.
	// EditDefaultsOnly: Defined in the data asset configuration.
	// BlueprintReadOnly: Can be read by Blueprint graphs for UI display purposes.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();
	
	// Detailed description of what this attribute does, shown in tooltips or info panels.
	// Uses FText for localization support (e.g., "The amount of damage you can take before dying").
	// EditDefaultsOnly: Set once in the data asset.
	// BlueprintReadOnly: Blueprints can read this for displaying tooltips.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();
	
	// The current runtime value of this attribute (e.g., current health = 75.5).
	// This is NOT set in the editor - it's populated at runtime by querying the AttributeSet.
	// BlueprintReadOnly: Blueprints can read the current value for display in UI.
	// Not marked EditDefaultsOnly because this value changes during gameplay.
	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.f;
};

/**
 * 
 */
UCLASS()
class FOX_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	// Searches the AttributeInformation array for an entry matching the provided AttributeTag.
	// Returns the matching FFoxAttributeInfo struct, or an empty struct if not found.
	// If bLogNotFound is true and no match is found, logs an error message.
	FFoxAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;
	
	// Array of instances of the FFoxAttributeInfo struct defined at the top of this file
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FFoxAttributeInfo> AttributeInformation;
};
