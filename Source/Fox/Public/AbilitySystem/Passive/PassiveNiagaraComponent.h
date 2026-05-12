// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

class UFoxAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class FOX_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
public:
	
	// Constructor
	UPassiveNiagaraComponent();

	// The gameplay tag that identifies which passive ability this Niagara component is associated with
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;

protected:
		
	/**
	 * Called when the component begins play during actor initialization.
	 * 
	 * This function attempts to bind the OnPassiveActivate callback function to the owner's AbilitySystemComponent's 
	 * (ASC) ActivatePassiveEffect delegate. When the ActivatePassiveEffect delegate broadcasts, it will call the 
	 * OnPassiveActivate callback function to handle passive ability activation events. 
	 */
	virtual void BeginPlay() override;

	/**
	 * Callback function bound to the AbilitySystemComponent's ActivatePassiveEffect delegate. This function is 
	 * invoked when the ActivatePassiveEffect delegate broadcasts, which occurs when any passive ability is 
	 * activated or deactivated on the owner's ASC.
	 * 
	 * This function filters activation events by comparing the broadcasted ability tag against this component's 
	 * PassiveSpellTag. When a matching tag is detected, it activates or deactivates the Niagara particle system 
	 * accordingly to provide visual feedback for the passive ability's state.
	 * 
	 * @param AbilityTag The gameplay tag identifying which passive ability was activated/deactivated
	 * @param bActivate True if the passive ability is being activated, false if it's being deactivated
	 */
	void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);

	/**
	 * Activates this Niagara component if the associated passive ability identified by PassiveSpellTag is already 
	 * equipped/active on the provided AbilitySystemComponent. This function is typically called during initialization 
	 * to ensure the visual effect is displayed when the component is created after the passive ability has already 
	 * been activated (e.g., when spawning into a level with an already-equipped passive ability).
	 * 
	 * @param FoxASC The AbilitySystemComponent to query for the passive ability's equipped/active state
	 */
	void ActivateIfEquipped(UFoxAbilitySystemComponent* FoxASC);
};
