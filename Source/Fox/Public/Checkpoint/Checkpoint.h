// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Checkpoint.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class FOX_API ACheckpoint : public APlayerStart
{
	GENERATED_BODY()
public:

	// Constructor that initializes the checkpoint actor with custom component setup using the provided object initializer
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

protected:

	// Callback function triggered when an actor overlaps with the checkpoint's sphere collision component, used to detect when the player reaches the checkpoint
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when the checkpoint actor begins play in the level, used to initialize components and bind overlap events
	virtual void BeginPlay() override;

	// Blueprint implementable event that is called when the checkpoint is reached by the player, allowing blueprints to handle visual feedback using the provided dynamic material instance
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);\

	// Handles the visual glow effects for the checkpoint by creating and configuring a dynamic material instance for the checkpoint mesh
	void HandleGlowEffects();
private:

	// The static mesh component that represents the visual geometry of the checkpoint in the game world
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	// The sphere collision component used to detect when the player character overlaps with the checkpoint trigger area
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
