// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "FoxPlayerController.generated.h"

class UNiagaraSystem;
class UDamageTextComponent;
class USplineComponent;
class UFoxAbilitySystemComponent;
class UFoxInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 
 */
UCLASS()
class FOX_API AFoxPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AFoxPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	// Client RPC. If called on the server it will be executed on the server if the controlling player is local but if the
	// controlling player is remote it will be executed on the client. This function constructs the 
	// DamageTextComponent and displays it on the screen. TargetCharacter is the actor we want to show this component above
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	
	// IMC set in the blueprint for this class
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> FoxContext;
	
	// Input action for character movement set in the blueprint for this class.
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	// Input action set in the blueprint for this class
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;
	
	// Input action callbacks
	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };
	bool bShiftKeyDown;
	
	void Move(const FInputActionValue& InputActionValue);
	
	void CursorTrace();
	
	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;
	
	FHitResult CursorHit;
	
	// Callback functions for handling ability input events (pressed, released, and held) identified by gameplay tags
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	// Data asset set from the blueprint.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UFoxInputConfig> InputConfig;
	
	// Variable to hold the ASC so that we only have to cast once
	UPROPERTY()
	TObjectPtr<UFoxAbilitySystemComponent> FoxAbilitySystemComponent;
	
	// Function to get th ASC
	UFoxAbilitySystemComponent* GetASC();
	
	/* 
	 * Auto run variables - These will need to be removed idk why anyone would want this type of movement
	*/
	
	// Cached destination of a location the player clicks on
	FVector CachedDestination = FVector::ZeroVector;
	
	// How long the player has pressed the mouse button before releasing it
	float FollowTime = 0.f;
	
	// Max length of LMB click press that is considered to be a short press
	float ShortPressThreshold = 0.5f;
	
	// When true the character should be auto running
	bool bAutoRunning = false;
	
	// Signifies whether object under the cursor is a enemy that should be targeted or if a click should cause auto 
	// running
	bool bTargeting = false;
	
	// Distance from the destination at which the auto running can stop
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	
	// Allows us to generate a smooth curve out of a set of vector world locations
	UPROPERTY(VisibleAnywhere)
	USplineComponent* Spline;
	
	// Niagara particle system that plays at the location where the player clicks for visual feedback. The value for this
	// variable is set in the player controller BP
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;
	
	void AutoRun();
	
	/*
	 * end Auto run variables
	*/
	
	// Variable to hold a child of the DamageTextComponent class. This is set in the player controller BP
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
};
