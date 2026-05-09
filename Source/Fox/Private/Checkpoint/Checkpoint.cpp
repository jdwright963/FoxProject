// Copyright TryingToMakeGames


#include "Checkpoint/Checkpoint.h"

#include "Components/SphereComponent.h"

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
}

void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Verifies that the overlapping actor is the player character by checking for the "Player" tag
	if (OtherActor->ActorHasTag(FName("Player")))
	{
		// Triggers the visual glow effects and disables the checkpoint trigger now that the player has reached this checkpoint
		HandleGlowEffects();
	}
}

void ACheckpoint::BeginPlay()
{
	// Calls the parent class (AActor) BeginPlay implementation to ensure proper initialization of base actor functionality when the game starts
	Super::BeginPlay();

	// Binds the OnSphereOverlap function to the sphere component's overlap event, enabling detection when the player enters the checkpoint trigger volume
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnSphereOverlap);
}

void ACheckpoint::HandleGlowEffects()
{
	// Disables collision on the sphere trigger to prevent the player from activating the checkpoint multiple times after it has been reached
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Creates a dynamic material instance from the checkpoint mesh's first material slot, allowing runtime modifications to material parameters for visual glow effects
	UMaterialInstanceDynamic* DynamicMaterialInstace = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);

	// Applies the newly created dynamic material instance to the mesh's first material slot, replacing the static material with the dynamic version
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstace);

	// Calls the blueprint-implementable CheckpointReached event, passing the dynamic material instance to enable visual glow animations and effects
	CheckpointReached(DynamicMaterialInstace);
}
