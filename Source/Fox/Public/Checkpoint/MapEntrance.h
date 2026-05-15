// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Checkpoint/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()
public:
	
	// Constructor that initializes the map entrance actor with custom component setup using the provided object initializer
	// The FObjectInitializer parameter is a helper object provided by Unreal Engine during actor construction that allows
	// configuring and creating components, setting default values, and performing other initialization tasks before the
	// actor is fully constructed. It's passed by const reference and used internally by Unreal's object creation system.
	AMapEntrance(const FObjectInitializer& ObjectInitializer);

	/* Highlight Interface */
	
	// Applies visual highlighting effects to the map entrance mesh to indicate it can be interacted with for level transition.
	// We override this method to provide custom highlighting behavior for map entrances.
	virtual void HighlightActor_Implementation() override;
	
	/* Highlight Interface */

	/* Save Interface */
	
	// Loads the map entrance's saved state from disk, restoring actor properties and triggering visual updates for entrance availability
	// This method is overridden to handle custom loading logic specific to map entrances.
	virtual void LoadActor_Implementation() override;
	
	/* end Save Interface */
	
	// Soft reference to the target world/map that will be loaded when the player enters this map entrance
	// Configured in editor to specify which level to transition to
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	// Tag identifier used to locate the specific PlayerStart actor in the destination map where the player should spawn
	// Matches against PlayerStart tags in the target level to determine the exact spawn location
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;
	
protected:
	
	// Callback function triggered when an actor overlaps with the map entrance's sphere collision component
	// OverlappedComponent: The sphere component that was overlapped (this entrance's collision sphere)
	// OtherActor: The actor that overlapped with the sphere component, typically the player character
	// OtherComp: The primitive component on the other actor that caused the overlap
	// OtherBodyIndex: Index of the overlapping body if the other component has multiple bodies
	// bFromSweep: True if the overlap was detected during a "Sweep" (continuous physical movement).
	// When true, the SweepResult contains valid data like the exact impact point and surface normal.
	// When false, the overlap occurred without a movement path (e.g., via teleportation, spawning, 
	// or a simple position update), and the SweepResult should be considered empty/invalid.
	// SweepResult: Detailed hit result information if the overlap occurred during a sweep operation
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
								 const FHitResult& SweepResult) override;
};
