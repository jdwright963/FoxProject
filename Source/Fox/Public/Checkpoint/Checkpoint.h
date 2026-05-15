// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Fox/Fox.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/HighlightInterface.h"
#include "Interaction/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class FOX_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()
public:

	// Constructor that initializes the checkpoint actor with custom component setup using the provided object initializer
	ACheckpoint(const FObjectInitializer& ObjectInitializer);
	
	/* Save Interface */

	// Returns false to indicate that this checkpoint's transform (position, rotation, scale) should not be loaded from save data
	// Checkpoints remain at their original level-placed positions regardless of save state
	virtual bool ShouldLoadTransform_Implementation() override { return false; };

	// Loads the checkpoint's saved state from disk, restoring the bReached flag and updating visual effects accordingly
	virtual void LoadActor_Implementation() override;
	
	/* end Save Interface */
	
	// Boolean flag indicating whether this checkpoint has been reached by the player, automatically saved to disk
	// BlueprintReadOnly: Allows Blueprint scripts to read this value and modify it
	// SaveGame: Marks this property to be automatically serialized and saved when the game is saved
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	// Editor-configurable flag that controls whether the OnSphereOverlap callback should be bound to the sphere component's
	// overlap events during BeginPlay. This allows us to implement a different OnSphereOverlap function
	// in some blueprints that are derived from this class that require custom overlap handling but use the OnSphereOverlap function
	// from this class for other blueprints that are derived from this class.
	UPROPERTY(EditAnywhere)
	bool bBindOverlapCallback = true;
	
protected:

	// Callback function triggered when an actor overlaps with the checkpoint's sphere collision component, used to 
	// detect when the player reaches the checkpoint
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when the checkpoint actor begins play in the level, used to initialize components and bind overlap events
	virtual void BeginPlay() override;
	
	/* Highlight Interface */

	// Sets the destination location for player movement when this checkpoint is clicked, directing the player to the 
	// checkpoint's MoveToComponent position
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;

	// Applies visual highlighting effects to the checkpoint mesh using custom depth stencil rendering to indicate it 
	// can be interacted with 
	virtual void HighlightActor_Implementation() override;

	// Removes visual highlighting effects from the checkpoint mesh by disabling custom depth stencil rendering when no
	// longer targeted
	virtual void UnHighlightActor_Implementation() override;

	/* end Highlight Interface */

	// Scene component that defines the exact world position where the player character should navigate to when clicking
	// on this checkpoint, allowing precise control over the final destination point separate from the checkpoint's root location
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;

	// Custom depth stencil value used for visual highlighting effects when the checkpoint is targeted, defaults to 
	// CUSTOM_DEPTH_TAN to render the checkpoint with a tan-colored outline via post-process custom depth rendering
	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

	// Blueprint implementable event that is called when the checkpoint is reached by the player, allowing blueprints to
	// handle visual feedback using the provided dynamic material instance
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	// Handles the visual glow effects for the checkpoint by creating and configuring a dynamic material instance for the checkpoint mesh
	UFUNCTION(BlueprintCallable)
	void HandleGlowEffects();
	
	// The static mesh component that represents the visual geometry of the checkpoint in the game world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;
	
	// The sphere collision component used to detect when the player character overlaps with the checkpoint trigger area
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
