// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

// Declaring delegate type named FMouseTargetDataSignature that broadcasts a struct FGameplayAbilityTargetDataHandle
// that contains the HitResult for a trace under the mouse and this contains the value of the mouse cursor location.
// Callback function that bind to this delegate have to take one parameter of type FGameplayAbilityTargetDataHandle
// named DataHandle
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 
 */
UCLASS()
class FOX_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	
	// This is an asynchronous meaning it can have multiple output execution pins that only execute once some event 
	// occurs. To create these pins we create delegate variables on this class
	
	/*
	 * Called from the blueprint of a Gameplay Ability to create the target data task. The OwningAbility is passed in by
	 * default with a value of 'self' (the blueprint equivalent to to 'this') and this input pin is hidden
	 * This function is an internal function that is a factory that creates an instance of this task and returns it and
	 * that is the function that will be called by the blueprint. The function name in blueprint is TargetDataUnderMouse
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);
	
	/*
	 * Delegate instance that can be bound to in blueprints to receive the mouse cursor location data.
	 * This variable becomes an output execution pin on the node for this task (TargetDataUnderMouse) in the blueprint, 
	 * which is executed when this delegate is broadcasted. The Data parameter in the delegate type definition also 
	 * becomes an output pin (FVector), which will only have valid data once a broadcast occurs.
	*/
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;
	
private:
	
	// Activates the task and its functionality is in this function
	virtual void Activate() override;
	
	// Sends the mouse cursor data (the location of the mouse cursor) to the server. This function is only called when we are locally controlled. 
	// If we are on the server and locally controlled we are broadcasting the ValidData delegate. If we are on a client
	// and we are locally controlled we are broadcasting the ValidData delegate and sending the data to the server
	void SendMouseCursorData();
	
	// Callback bound to the AbilityTargetDataSetDelegate so it must take input parameters required for the delegate
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
