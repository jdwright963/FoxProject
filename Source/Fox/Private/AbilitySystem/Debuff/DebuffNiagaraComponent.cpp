// Copyright TryingToMakeGames


#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	// Whether the component is activated at creation or must be explicitly activated. Set to false here.
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	// Call the parent class BeginPlay to ensure proper initialization of the Niagara component
	Super::BeginPlay();

	// Attempt to cast the owning actor of this component to ICombatInterface to access combat-related functionality
	// This interface provides access to ASC registration and death event delegates
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());

	// Attempt to retrieve the Ability System Component (ASC) from the owning actor using Blueprint library helper
	// If successful, the ASC pointer is stored in the ASC variable for immediate registration
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		// Register for gameplay tag events on the ASC to monitor when the debuff tag is added or removed
		// RegisterGameplayTagEvent returns a delegate that we can bind our callback function to
		// EGameplayTagEventType::NewOrRemoved triggers callbacks when the tag count changes from 0 to non-zero or vice versa
		// AddUObject binds the DebuffTagChanged function to be called when the tag event fires.
		// It takes two parameters: 'this' (the object instance that will listen for the event) and the memory address 
		// of the callback function (&UDebuffNiagaraComponent::DebuffTagChanged). This creates a "UObject-safe" 
		// binding, meaning the delegate will automatically unbind or skip execution if 'this' is garbage collected.
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	// If the ASC is not immediately available but the owner implements CombatInterface
	// This handles cases where the ASC is initialized after BeginPlay (common in multiplayer scenarios)
	else if (CombatInterface)
	{
		// Retrieve the delegate that broadcasts when the ASC becomes available on the owning actor using the getter
		// function we defined in CombatInterface and implemented in classes that derive from it.
		// AddWeakLambda binds a lambda function that will be called when the ASC is registered.
		// It takes two parameters: 'this' (the object context used to track the lifetime of the binding) 
		// and the lambda expression itself ([this](UAbilitySystemComponent* InASC)). 
		// The InASC input parameter is the Ability System Component instance passed by the delegate 
		// when it broadcasts, providing immediate access to the ASC as soon as it is registered.
		// Using WeakLambda ensures the binding is automatically cleared if this component is destroyed, 
		// preventing the lambda from executing on a dangling pointer, while the 'this' capture allows 
		// the lambda to access the component's members.
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
		{
			// Once the ASC is available, register for the debuff tag events (same logic as the immediate case above)
			// InASC is the newly registered Ability System Component passed by the delegate
			// This ensures we can monitor debuff tags even when the ASC is initialized late
			InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
		});
	}
	// Check if the owning actor implements CombatInterface to access death event functionality
	// This is a separate check because we want to bind to death events regardless of ASC availability
	if (CombatInterface)
	{
		// Retrieve the delegate that broadcasts when the owning actor dies using the getter
		// function we defined in CombatInterface and implemented in classes that derive from it.
		// AddDynamic binds the OnOwnerDeath UFUNCTION to be called when the death event is triggered.
		// This ensures the Niagara effect is properly deactivated when the actor dies.
		// We use AddDynamic because FOnDeath is a Dynamic Multicast Delegate; unlike standard delegates 
		// which use AddUObject, dynamic delegates allow for Blueprint interoperability and require 
		// using this specific macro to bind functions marked with the UFUNCTION() specifier. 
		// It takes 'this' (the target object instance that will receive the callback) and the 
		// address of the callback function. This object context allows the delegate to 
		// automatically skip execution if the object has been garbage collected.
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// Check if the debuff tag count is greater than zero, indicating the debuff has been applied to the owning actor
	// NewCount represents the current number of instances of this gameplay tag on the actor's ASC
	if (NewCount > 0)
	{
		// Activate the Niagara particle effect to visually indicate the debuff is active on the owning actor
		// This function is inherited from UNiagaraComponent and is used to start the Niagara effect
		Activate();
	}
	// If the debuff tag count is zero or less, the debuff has been removed from the owning actor
	else
	{
		// Deactivate the Niagara particle effect since the debuff is no longer active on the owning actor
		// This function is inherited from UNiagaraComponent and is used to end the Niagara effect
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	// Deactivate the Niagara particle effect when the owning actor dies to ensure visual effects 
	// are properly cleaned up and don't continue playing on a dead actor
	// This function is inherited from UNiagaraComponent and is used to end the Niagara effect
	Deactivate();
}
