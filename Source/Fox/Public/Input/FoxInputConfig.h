// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "FoxInputConfig.generated.h"

// Struct our data asset will contain an array of
// Struct bodies are public automatically
USTRUCT(BlueprintType)
struct FFoxInputAction
{
	GENERATED_BODY()
	
	// Const means this cannot be changed at runtime but it can be changed in the blueprint for this data asset
	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * 
 */
UCLASS()
class FOX_API UFoxInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	
	// Returns UInputAction and not FFoxInput action so that we do not have to retrieve the UInputAction from the struct
	// Searches the AbilityInputActions array for an input action that has a specific tag
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;
	
	// Array of the structs defined above. The values are assigned in the blueprint. If we have a pointer or ref to 
	// This class then we can lookup in this array any of the input actions that have a specific tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FFoxInputAction> AbilityInputActions;
};
