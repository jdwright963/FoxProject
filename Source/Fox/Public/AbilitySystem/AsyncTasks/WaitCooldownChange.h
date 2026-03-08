// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"

// Forward declarations
class UAbilitySystemComponent;
struct FGameplayEffectSpec;

// Declares a delegate type named FCooldownChangeSignature for broadcasting a float value representing the time 
// remaining on a cooldown
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 
 */

// BlueprintType: Makes this class available as a variable type in Blueprint (e.g., you can create Blueprint variables 
// of type UWaitCooldownChange). Without this, the class would only be usable through its functions but couldn't be 
// stored or passed around as a variable in Blueprint.
//
// ExposedAsyncProxy = "AsyncTask": This meta specifier works in conjunction with BlueprintInternalUseOnly functions 
// to control how the async task node appears in Blueprint. It specifies the name of the output pin that returns the 
// async task object itself. In this case, the pin will be labeled "AsyncTask" and will output a reference to the 
// UWaitCooldownChange object. This return value pin allows you to:
// 1. Store the async task object in a variable for later use
// 2. Call EndTask() on it to manually stop the async operation
// 3. Pass it to other functions that might need to interact with the async task
// Without this specifier, the async node would still work, but the return value pin would have a generic or default name
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class FOX_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	
	// Instance of the FCooldownChangeSignature delegate type declared above. This can be bound to in blueprint. This 
	// delegate instance broadcasts when the cooldown starts
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;

	// Instance of the FCooldownChangeSignature delegate type declared above. This can be bound to in blueprint. This 
	// delegate instance broadcasts when the cooldown ends
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;
	
	
	/*
	 * Creates and returns a new instance of this class, UWaitCooldownChange, that monitors the specified cooldown tag on the given 
	 * ability system component. When a gameplay effect with the cooldown tag is applied, the CooldownStart delegate 
	 * broadcasts with the remaining time. When the cooldown tag is removed (count reaches zero), the CooldownEnd 
	 * delegate broadcasts.
	 *
	 * Why is this function static instead of just being a regular constructor? 
	 * 
	 * Constructors are "invisible" to Blueprint. Static factory functions like this one act as 
	 * "visible constructors" that Blueprint can see and call. When you call WaitForCooldownChange in Blueprint, it 
	 * internally uses NewObject<UWaitCooldownChange>() to create the instance (see the .cpp file), initializes it 
	 * with your parameters, sets up the necessary callbacks, and returns the configured object - essentially doing 
	 * everything a constructor would do, but in a way that Blueprint can access.
	 *
	 * The "BlueprintInternalUseOnly = true" meta specifier is a special flag that fundamentally changes how this 
	 * function appears and behaves in Blueprint:
	 *
	 * 1. NODE APPEARANCE: Instead of appearing as a regular function call node, Blueprint's code generator recognizes 
	 *    this meta tag and creates a special "async task" node with a unique visual appearance. This node displays 
	 *    multiple execution pins (one for each UPROPERTY BlueprintAssignable delegate - in this case, CooldownStart 
	 *    and CooldownEnd) rather than a single output execution pin.
	 *
	 * 2. EXECUTION FLOW: The generated node doesn't complete immediately like normal function calls. Instead, it 
	 *    spawns the async task and continues executing the Blueprint graph while the task runs in the background. 
	 *    When events occur (cooldown starts/ends), the appropriate delegate fires and triggers execution from the 
	 *    corresponding output pin on the async node.
	 *
	 * 3. AUTOMATIC DELEGATE BINDING: Blueprint's compiler automatically generates hidden code that discovers all 
	 *    BlueprintAssignable delegate properties on the returned UObject (CooldownStart and CooldownEnd in this case) 
	 *    and creates output execution pins for each one. When you connect nodes to these output pins in Blueprint, 
	 *    the compiler automatically binds those Blueprint execution chains to the corresponding C++ delegates.
	 *
	 * 4. LIFECYCLE MANAGEMENT: The BlueprintInternalUseOnly system expects the returned UObject to manage its own 
	 *    lifecycle. The object should call SetReadyToDestroy() when it's finished, which signals to Unreal's garbage 
	 *    collection system that the object can be cleaned up. This is why EndTask() calls both SetReadyToDestroy() 
	 *    and MarkAsGarbage().
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag);

	// Manually ends the async task by unregistering all delegate callbacks from the ability system component and 
	// marking this object for garbage collection
	UFUNCTION(BlueprintCallable)
	void EndTask();
protected:

	// Variable to store the ability system component
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	// Variable to store the cooldown tag
	FGameplayTag CooldownTag;

	// Callback function invoked when the cooldown tag count changes (added or removed). Broadcasts CooldownEnd 
	// delegate when the tag count reaches zero, indicating the cooldown has finished
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
	
	// Callback function invoked when a new gameplay effect is added to the ability system component. Checks if the 
	// applied effect contains the cooldown tag and broadcasts CooldownStart delegate with the remaining cooldown time
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
};
