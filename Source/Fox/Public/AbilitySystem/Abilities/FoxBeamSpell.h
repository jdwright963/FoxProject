// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/FoxDamageGameplayAbility.h"
#include "FoxBeamSpell.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UFoxBeamSpell : public UFoxDamageGameplayAbility
{
	GENERATED_BODY()
public:
	
	/**
	 * Stores mouse cursor trace hit information for the beam spell targeting.
	 * If the trace hits a blocking object, stores the impact location and hit actor.
	 * If no blocking hit occurs, cancels the ability.
	 * @param HitResult The result of the mouse cursor trace
	 */
	UFUNCTION(BlueprintCallable)
	void StoreMouseDataInfo(const FHitResult& HitResult);
	
	/**
	 * Caches the player controller and character references from the current actor info.
	 * Stores them in OwnerPlayerController and OwnerCharacter for later use during ability execution.
	 *  CurrentActorInfo: Contains information about the actor/avatar executing this ability, including:
	 *	 - OwnerActor: The actor that owns the AbilitySystemComponent (e.g., PlayerState)
	 *	 - AvatarActor: The physical actor in the world (e.g., Character)
	 *	 - PlayerController: The controller for player-controlled avatars
	 *   - AbilitySystemComponent: The ASC that granted this ability
	 */
	UFUNCTION(BlueprintCallable)
	void StoreOwnerVariables();
	
	/**
	 * Performs a trace from the owner character's weapon tip socket towards the beam target location
	 * to find the first valid target actor for the beam spell. Updates MouseHitActor and MouseHitLocation
	 * with the results of this trace, which will be used as the initial target point for the beam.
	 * 
	 * We do this because an enemy could be standing between the character executing this ability and the enemy
	 * they originally targeted with the mouse cursor and we do not want this ability to go through the enemy
	 * inbetween (ignoring them) and hitting the initial target
	 * 
	 * @param BeamTargetLocation The world location to trace towards, typically set from the mouse cursor hit location
	 */
	UFUNCTION(BlueprintCallable)
	void TraceFirstTarget(const FVector& BeamTargetLocation);
	
	/**
	 * Finds and stores additional target actors within range for the beam spell's shock propagation effect.
	 * Starting from the initial beam target, searches for nearby valid actors up to MaxNumShockTargets,
	 * excluding actors that have already been targeted. This creates a chaining/propagating beam effect
	 * that can jump between multiple enemies.
	 * 
	 * @param OutAdditionalTargets Array to be populated with additional valid target actors found within shock range
	 */
	UFUNCTION(BlueprintCallable)
	void StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets);
protected:

	/** The world location where the mouse cursor trace hit a blocking object, used as the target location for the beam spell */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	FVector MouseHitLocation;

	/** The actor that was hit by the mouse cursor trace, represents the potential target of the beam spell */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<AActor> MouseHitActor;

	/** Cached reference to the player controller that owns this ability, used for accessing player-specific functionality */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<APlayerController> OwnerPlayerController;
	
	/** Cached reference to the character avatar executing this ability, used for accessing character-specific functionality like mesh sockets and movement */
	UPROPERTY(BlueprintReadWrite, Category = "Beam")
	TObjectPtr<ACharacter> OwnerCharacter;
	
	/** Maximum number of additional targets the beam can chain to after hitting the initial target, used to limit the shock propagation effect */
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	int32 MaxNumShockTargets = 5;
};
