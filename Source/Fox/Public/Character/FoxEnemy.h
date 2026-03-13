// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Character/FoxCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "FoxEnemy.generated.h"

class AFoxAIController;
class UBehaviorTree;
class UWidgetComponent;
/**
 * 
 */
UCLASS()
class FOX_API AFoxEnemy : public AFoxCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
	
	// The constructor for this class
	AFoxEnemy();
	
	// This is called when the enemy is possessed by a controller
	virtual void PossessedBy(AController* NewController) override;
	
	/** Enemy Interface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/** end Enemy Interface */
	
	/**	Combat Interface */
	
	// Function to get this enemy's player level. This function is declared in PlayerInterface.h and overriden here
	virtual int32 GetPlayerLevel_Implementation() override;
	
	// Function for character death from the CombatInterface and overridden here
	// This handles what happens on the server when the character dies. Takes a DeathImpulse parameter which is a vector
	// representing the force and direction to apply to the character's ragdoll physics upon death.
	virtual void Die(const FVector& DeathImpulse) override;
	
	// Function to set the combat target for the enemy
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	
	// Function to get the combat target for the enemy
	virtual AActor* GetCombatTarget_Implementation() const override;
	
	/**	end Combat Interface */
	
	// Instance of the delegate type created in UI/WidgetController/OverlayWidgetController.h
	// This delegate is used to notify UI elements about health changes and broadcasts the new health value
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	// Instance of the delegate type created in UI/WidgetController/OverlayWidgetController.h
	// This delegate is used to notify UI elements about MaxHealth changes and broadcasts the new MaxHealth value
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	// Callback function that is called by the engine defined ASC delegate that broadcasts when the GameplayTags on the 
	// ASC change. This function is bount to only be called when the HitReact tag is added or removed completely (Not
	// just a single instance) from the ASC
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	// Variable that is true if the enemy is currently hit reacting
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;
	
	// Variable that is the lifespan of this enemy's dead body after which it will dissapear
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f; 
	
	// Variable that is the enemy's combat target. This is used for the motion warping target for rotating to face the target
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;
	
protected:
	
	// Called when the actor begins play. Performs initial setup for the enemy including binding callbacks and initializing systems
	virtual void BeginPlay() override;

	// Initializes the Ability System Component with this actor's information (owner, avatar, etc.)
	virtual void InitAbilityActorInfo() override;

	// Applies the default attribute values to this character using gameplay effects based on character class and level
	virtual void InitializeDefaultAttributes() const override;
	
	// Callback function that is called by the engine defined ASC delegate that broadcasts when the GameplayTags on the 
	// ASC change. This function is bound to only be called when the Stun tag is added or removed completely (Not
	// just a single instance) from the ASC. Inherited from AFoxCharacterBase and overridden here
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;
	
	// Variable that defines the enemy's level. This value is used by the CharacterClassInfo system to scale the 
	// enemy's attributes (Primary, Secondary, and Vital) based on the character class. Higher levels result in 
	// stronger attributes through the InitializeDefaultAttributes() function which applies scaled gameplay effects
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;
	
	// Variable for the widget component to display health bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
	TObjectPtr<UWidgetComponent> HealthBar;
	
	// Variable for the Behavior Tree that will be used by this enemy. This is set in the blueprint
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// Variable for the AI Controller that will be used by this enemy.
	UPROPERTY()
	TObjectPtr<AFoxAIController> FoxAIController;
};
