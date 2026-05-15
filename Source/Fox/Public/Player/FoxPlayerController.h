// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "FoxPlayerController.generated.h"

class IHighlightInterface;
class AMagicCircle;
class UNiagaraSystem;
class UDamageTextComponent;
class USplineComponent;
class UFoxAbilitySystemComponent;
class UFoxInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

// Enum representing the current targeting state of the player controller for cursor-based interactions
// TargetingEnemy - Cursor is over an actor that implements the EnemyInterface (hostile target)
// TargetingNonEnemy - Cursor is over an actor that doesn't implement the EnemyInterface (neutral/friendly target)
// NotTargeting - Cursor is not over any valid targetable actor
enum class ETargetingStatus : uint8
{
	TargetingEnemy,
	TargetingNonEnemy,
	NotTargeting
};

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
	
	// Blueprint callable function that displays the magic circle decal for targeting and ability placement
	// DecalMaterial - Optional material to override the default decal appearance, defaults to the material set in the blueprint if nullptr
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	// Blueprint callable function that hides the magic circle decal, called when targeting or ability placement is complete
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();
	
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
	
	// A dual-pointer container that stores both the underlying UObject and its 
	// IHighlightInterface implementation. This ensures the actor is protected 
	// from Garbage Collection while providing direct, high-performance access 
	// to interface methods without the need for repeated casting.
	// Stores the actor that was highlighted in the previous cursor trace tick, used to unhighlight it when the cursor moves away
	//TScriptInterface<IHighlightInterface> LastActor;

	// A dual-pointer container that stores both the underlying UObject and its 
	// IHighlightInterface implementation. This ensures the actor is protected 
	// from Garbage Collection while providing direct, high-performance access 
	// to interface methods without the need for repeated casting.
	// Stores the actor currently under the cursor in this frame's trace, used to highlight it if it implements the HighlightInterface
	//TScriptInterface<IHighlightInterface> ThisActor;
	
	// Stores the actor that was under the cursor in the previous frame, used to unhighlight it when the cursor moves away
	TObjectPtr<AActor> LastActor;

	// Stores the actor currently under the cursor in this frame, used to highlight it if it implements the HighlightInterface
	TObjectPtr<AActor> ThisActor;
	
	FHitResult CursorHit;
	
	// Static helper function that highlights the given actor if it implements the HighlightInterface
	static void HighlightActor(AActor* InActor);

	// Static helper function that unhighlights the given actor if it implements the HighlightInterface
	static void UnHighlightActor(AActor* InActor);
	
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
	
	// Tracks what type of actor (enemy, non-enemy, or none) is currently under the cursor based on the latest cursor trace
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;
	
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
	
	// A variable that stores a subclass of AMagicCircle. We set the value in the Blueprint 
	// to be a specific Blueprint class derived from AMagicCircle. This is the actor 
	// used to visualize ability placement and targeting in the world.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	// Spawned instance of the magic circle actor that follows the cursor for targeting and ability placement
	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;

	// Updates the magic circle's world position to follow the cursor's hit location on surfaces
	void UpdateMagicCircleLocation();
};
