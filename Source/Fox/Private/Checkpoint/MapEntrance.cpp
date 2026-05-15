// Copyright TryingToMakeGames


#include "Checkpoint/MapEntrance.h"

#include "Components/SphereComponent.h"
#include "Game/FoxGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"


AMapEntrance::AMapEntrance(const FObjectInitializer& ObjectInitializer)
	// Calls the parent class constructor and passes the ObjectInitializer up the chain 
	// so that the base class can initialize its own components and properties correctly.
	: Super(ObjectInitializer)
{
	// Attaches the sphere collision component to the MoveToComponent so that the overlap trigger follows the destination point
	Sphere->SetupAttachment(MoveToComponent);
}

void AMapEntrance::HighlightActor_Implementation()
{
	// Enables custom depth stencil rendering on the checkpoint mesh to create a visual highlight effect, allowing the 
	// mesh to be rendered with a colored outline in post-processing using the CustomDepthStencilOverride value
	CheckpointMesh->SetRenderCustomDepth(true);
}

void AMapEntrance::LoadActor_Implementation()
{
	// Do nothing when loading a Map Entrance since we do not want the parent implementation to run (because it handles
	// glow effects and we do not want the MapEntrance to glow) we want this empty function to run instead
}

void AMapEntrance::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checks if the overlapping actor implements the PlayerInterface to verify it is the player character before processing the map entrance trigger
	if (OtherActor->Implements<UPlayerInterface>())
	{
		// Marks this map entrance as reached so this save game variable can be saved and restored in future game sessions
		bReached = true;

		// Attempts to retrieve the current game mode as FoxGameModeBase to access the world state saving functionality
		if (AFoxGameModeBase* FoxGM = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			// Saves the current world state including all actor states before transitioning to the destination map to preserve game progress
			// GetWorld() retrieves a pointer to the current UWorld object representing the entire game world and all its actors
			// DestinationMap.ToSoftObjectPath().GetAssetName() extracts the asset name string from the destination map's 
			// soft object pointer to use as the save file identifier
			FoxGM->SaveWorldState(GetWorld(), DestinationMap.ToSoftObjectPath().GetAssetName());
		}

		// Saves the player's current progress with the destination player start tag so the player spawns at the correct
		// location in the new map. The value of DestinationPlayerStartTag is set in the editor in blueprints that are 
		// derived from this class
		IPlayerInterface::Execute_SaveProgress(OtherActor, DestinationPlayerStartTag);

		// Opens the destination map level using the soft object pointer to initiate the level transition
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	}
}
