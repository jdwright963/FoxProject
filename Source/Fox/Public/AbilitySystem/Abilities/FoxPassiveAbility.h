// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxGameplayAbility.h"
#include "FoxPassiveAbility.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxPassiveAbility : public UFoxGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * Activates this passive ability and registers it to listen for deactivation requests.
	 * Binds the ReceiveDeactivate callback functio to the FoxAbilitySystemComponent's DeactivatePassiveAbility 
	 * delegate so this ability can be remotely deactivated when a matching gameplay tag is broadcast.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * Callback invoked when the ASC broadcasts a passive ability deactivation request with the DeactivatePassiveAbility delegate.
	 * Terminates this ability if the provided AbilityTag exactly matches any tag in this ability's AbilityTags container.
	 * @param AbilityTag The gameplay tag identifying which passive ability should be deactivated.
	 */
	void ReceiveDeactivate(const FGameplayTag& AbilityTag);
	
};
