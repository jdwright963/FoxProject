// Copyright TryingToMakeGames


#include "Checkpoint/Checkpoint.h"

#include "Components/SphereComponent.h"
#include "Game/FoxGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer)
	// Calls the parent class (AActor) constructor, passing the ObjectInitializer to properly initialize the actor's base components and properties
	: Super(ObjectInitializer)
{
	// Disables per-frame tick updates for this actor since checkpoints are static and don't require continuous processing
	PrimaryActorTick.bCanEverTick = false;

	// Creates the static mesh component that represents the visual appearance of the checkpoint
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	
	// Attaches the checkpoint mesh to the actor's root component for proper hierarchy and transformation
	CheckpointMesh->SetupAttachment(GetRootComponent());
	
	// Enables both query (overlap/trace) and physics collision for the mesh to make it physically interactive
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	// Sets the mesh to block all collision channels by default, making it a solid physical object
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);
	
	// Sets the custom depth stencil value used by post-processing materials to apply outline/highlight effects when 
	// the checkpoint is rendered with custom depth enabled
	CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);

	// Forces the rendering system to update the mesh's visual state immediately, ensuring the custom depth stencil 
	// value is applied to the rendered output
	CheckpointMesh->MarkRenderStateDirty();

	// Creates the sphere component that acts as a trigger volume to detect when the player enters the checkpoint area
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	
	// Attaches the sphere to the checkpoint mesh so it moves and transforms with the visual representation
	Sphere->SetupAttachment(CheckpointMesh);
	
	// Enables query-only collision for the sphere, allowing overlap detection without affecting physics simulation
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// Configures the sphere to ignore collision with all channels by default, preventing unwanted interactions
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// Specifically enables overlap events with the Pawn channel to detect when the player character enters the checkpoint trigger
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	// Creates a scene component that defines the precise location where the player character should move/teleport when interacting with this checkpoint
	MoveToComponent = CreateDefaultSubobject<USceneComponent>("MoveToComponent");

	// Attaches the move-to location component to the actor's root, allowing level designers to independently position the teleport destination relative to the checkpoint
	MoveToComponent->SetupAttachment(GetRootComponent());
}

void ACheckpoint::LoadActor_Implementation()
{
	// Checks if this checkpoint was previously reached before the game was saved, ensuring its activated state is preserved when loading
	if (bReached)
	{
		// Reapplies the visual glow effects to maintain the checkpoint's activated appearance after loading the saved game state
		HandleGlowEffects();
	}
}

void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checks if the overlapping actor implements the PlayerInterface to ensure only player-controlled characters can activate this checkpoint
	if (OtherActor->Implements<UPlayerInterface>())
	{
		// Marks this checkpoint as reached by setting the flag to true, ensuring the checkpoint's activated state persists through save/load operations
		bReached = true;
		
		// Retrieves the current game mode instance and casts it to AFoxGameModeBase to access custom save functionality
		if (AFoxGameModeBase* FoxGM = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			// Retrieves the UWorld object representing the current game world, providing access to map information and world context
			const UWorld* World = GetWorld();
			
			// Gets the current map/level name from the world, which is used to organize and identify save data for this specific level
			FString MapName = World->GetMapName();
			
			// Removes Unreal's internal streaming level prefix from the map name to get the clean, user-facing level
			// name for save file organization
			MapName.RemoveFromStart(World->StreamingLevelsPrefix);

			// Saves the current world state to persistent storage, including this checkpoint's reached status, enemy spawn 
			// volume states, and all other actors implementing SaveInterface. Parameters: GetWorld() provides the UWorld 
			// context containing all actors and game state to be saved, while MapName identifies which specific level's 
			// save data should be written to or updated in the save file.
			FoxGM->SaveWorldState(GetWorld(), MapName);
		}
		
		// Calls the SaveProgress function on the player character using the Execute_ static function pattern.
		// IPlayerInterface::Execute_SaveProgress is a static function automatically generated by Unreal's reflection system
		// when SaveProgress is declared as a BlueprintNativeEvent in the interface. The Execute_ prefix indicates this is
		// the entry point that handles both C++ and Blueprint implementations, dispatching to the appropriate version.
		// This passes the checkpoint's PlayerStartTag to save the player's current progress and respawn location.
		IPlayerInterface::Execute_SaveProgress(OtherActor, PlayerStartTag);
		
		// Triggers the visual glow effects and disables the checkpoint trigger now that the player has reached this checkpoint
		HandleGlowEffects();
	}
}

void ACheckpoint::BeginPlay()
{
	// Calls the parent class (AActor) BeginPlay implementation to ensure proper initialization of base actor functionality when the game starts
	Super::BeginPlay();
	
	// Checks if overlap callback binding is enabled, this allows us to implement a different OnSphereOverlap function
	// in some blueprints that are derived from this class that require custom overlap handling but use the OnSphereOverlap function
	// from this class for other blueprints that are derived from this class.
	if (bBindOverlapCallback)
	{
		// Binds the OnSphereOverlap function to the sphere component's overlap event, enabling detection when the player enters the checkpoint trigger volume
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnSphereOverlap);
	}
}

void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	// Retrieves the world-space location of the MoveToComponent and assigns it to the output parameter, providing the 
	// exact destination where the player should teleport when interacting with this checkpoint
	OutDestination = MoveToComponent->GetComponentLocation();
}

void ACheckpoint::HighlightActor_Implementation()
{
	// Only applies highlighting effects (when cursor is over checkpoint) to checkpoints that haven't been reached yet, 
	// preventing visual clutter and clearly indicating which checkpoints are still available for interaction
	if (!bReached)
	{
		// Enables custom depth rendering for the checkpoint mesh, allowing post-process materials to apply visual highlight 
		// effects like outlines when the player targets or hovers over this checkpoint
		CheckpointMesh->SetRenderCustomDepth(true);
	}
}

void ACheckpoint::UnHighlightActor_Implementation()
{
	// Disables custom depth rendering for the checkpoint mesh, removing any visual highlight effects like outlines when
	// the player no longer targets or hovers over this checkpoint
	CheckpointMesh->SetRenderCustomDepth(false);
}

void ACheckpoint::HandleGlowEffects()
{
	// Disables collision on the sphere trigger to prevent the player from activating the checkpoint multiple times after it has been reached
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Creates a dynamic material instance from the checkpoint mesh's first material slot, allowing runtime modifications to material parameters for visual glow effects
	UMaterialInstanceDynamic* DynamicMaterialInstace = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);

	// Applies the newly created dynamic material instance to the mesh's first material slot, replacing the static material with the dynamic version
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstace);

	// Calls the blueprint-implementable CheckpointReached event, passing the dynamic material instance to enable visual glow animations and effects defined in Blueprint subclasses
	CheckpointReached(DynamicMaterialInstace);
}
