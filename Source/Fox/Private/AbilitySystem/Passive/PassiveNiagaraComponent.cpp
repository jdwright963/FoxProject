// Copyright TryingToMakeGames


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	// Prevent automatic activation. The component will be activated when the associated passive ability is activated
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	// Call parent class BeginPlay to initialize the Niagara component
	Super::BeginPlay();
	
	// Attempt to retrieve the owner's (the actor that owns this component) AbilitySystemComponent using the blueprint 
	// library helper function and cast it to UFoxAbilitySystemComponent to access Fox-specific functionality
	// The if statement checks if the cast was successful and only proceeds if the cast was successful
	if (UFoxAbilitySystemComponent* FoxASC = Cast<UFoxAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		// Bind this component's OnPassiveActivate callback function to the ASC's ActivatePassiveEffect multicast 
		// delegate to receive notifications when passive abilities are activated or deactivated. The callback function
		// will be called whenever this delegate is broadcasted
		FoxASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
			
		// Immediately check if the passive ability associated with this component is already equipped and activate
		// the Niagara effect if it is, ensuring the visual effect is shown without waiting for a passive activation event
		ActivateIfEquipped(FoxASC);
	}
	// If the AbilitySystemComponent is not yet available, check if the owner implements the CombatInterface which 
	// provides a delegate for late ASC registration notifications
	else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		// Register a lambda callback to the OnASCRegistered delegate this callback will be called when the AbilitySystemComponent
		// is registered later (common occurence in multiplayer)
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC)
		{
			// Once the ASC of the actor that owns this component is registered, attempt to retrieve the ASC of the owner
			// again using the blueprint library helper function and cast it to UFoxAbilitySystemComponent to access 
			// Fox-specific functionality. The if statement checks if the cast was successful and only proceeds if the 
			// cast was successful
			if (UFoxAbilitySystemComponent* FoxASC = Cast<UFoxAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
			{
				// Bind this component's OnPassiveActivate callback function to the ASC's ActivatePassiveEffect multicast 
				// delegate to receive notifications when passive abilities are activated or deactivated. The callback function
				// will be called whenever this delegate is broadcasted
				FoxASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
				
				// Immediately check if the passive ability associated with this component is already equipped and activate
				// the Niagara effect if it is, ensuring the visual effect is shown without waiting for a passive activation event
				ActivateIfEquipped(FoxASC);
			}
		});
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	// Check if the ability tag, received from the ActivatePassiveEffect delegate that this callback function is bound to,
	// exactly matches this component's assigned PassiveSpellTag to ensure we only 
	// respond to the specific passive ability this Niagara component is associated with
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		// Check if the passive ability should be activated (bActivate is true) AND if this Niagara component is not 
		// already active to prevent redundant activation calls
		if (bActivate && !IsActive())
		{
			// Activate the Niagara particle system to visually represent the passive ability being active
			// This function is inherited from the parent class
			Activate();
		}
		// If the passive ability should be deactivated (bActivate is false) OR the component is already active
		else
		{
			// Deactivate the Niagara particle system to stop the visual effect when the passive ability is no longer active
			// This function is inherited from the parent class
			Deactivate();
		}
	}
}

void UPassiveNiagaraComponent::ActivateIfEquipped(UFoxAbilitySystemComponent* FoxASC)
{
	// Cache the startup abilities given flag to determine if the owner's initial abilities have been granted yet
	const bool bStartupAbilitiesGiven = FoxASC->bStartupAbilitiesGiven;
	
	// Only proceed with activation check if startup abilities have been given to avoid activating before abilities are initialized
	if (bStartupAbilitiesGiven)
	{
		// Check if the passive spell associated with this component currently has "Equipped" status in the ability system
		if (FoxASC->GetStatusFromAbilityTag(PassiveSpellTag) == FFoxGameplayTags::Get().Abilities_Status_Equipped)
		{
			// Activate the Niagara particle system since the passive ability is equipped and should display its visual effect
			Activate();
		}
	}
}
