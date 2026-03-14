// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"

void UFoxPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	// Call the parent class implementation to perform standard ability activation tasks
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Retrieve the Ability System Component from the avatar actor and cast it to our custom Fox ASC type.
	// This is necessary to access the DeactivatePassiveAbility delegate which is specific to our custom ASC.
	// We use GetAvatarActorFromActorInfo() to get the actor that owns this ability, then use the Blueprint Library
	// helper function to retrieve its ASC, and finally cast it to UFoxAbilitySystemComponent to access custom functionality
	// The if statement checks if the cast was successful and only proceeds if the ASC is of the correct type
	if (UFoxAbilitySystemComponent* FoxASC = Cast<UFoxAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		// Register this passive ability instance to listen for deactivation requests via the ASC's delegate.
		// When DeactivatePassiveAbility is broadcast with a gameplay tag, our ReceiveDeactivate method will be called,
		// allowing the ASC to remotely deactivate this passive ability
		FoxASC->DeactivatePassiveAbility.AddUObject(this, &UFoxPassiveAbility::ReceiveDeactivate);
	}
}

void UFoxPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	// Check if this ability's tag container contains an exact match for the deactivation tag.
	// The AbilityTags container is populated in the Blueprint asset (not in C++ code) by configuring the ability's tags.
	// HasTagExact ensures we only deactivate if the tag matches precisely (not only parent/child tag matching)
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		// Terminate this ability instance using the context variables inherited from UGameplayAbility:
		// - CurrentSpecHandle: A unique ID used by the ASC to identify this specific instance in the 'granted abilities' list.
		// - CurrentActorInfo: A cached pointer containing essential data about the Owner (Controller) and Avatar (Pawn).
		// - CurrentActivationInfo: Contains metadata about how the ability was triggered, including its networking and prediction state.
		//
		// The first boolean (true) ensures the server synchronizes this termination to the clients.
		// The second boolean (true) marks this as a cancellation (interruption) rather than a successful completion.
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
