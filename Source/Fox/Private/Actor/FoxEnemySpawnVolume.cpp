// Copyright TryingToMakeGames


#include "Actor/FoxEnemySpawnVolume.h"

#include "Actor/FoxEnemySpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Interaction/PlayerInterface.h"

// Sets default values
AFoxEnemySpawnVolume::AFoxEnemySpawnVolume()
{
 	// Set this actor to NOT call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create the box component that will serve as the overlap trigger volume for enemy spawning
	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	
	// Set the box component as the root component of this actor
	SetRootComponent(Box);
	
	// Enable query-only collision to detect overlaps without physically blocking objects
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	// Set the collision object type to WorldStatic for proper collision filtering
	Box->SetCollisionObjectType(ECC_WorldStatic);
	
	// Ignore collision responses for all channels by default
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// Enable overlap events specifically for the Pawn channel to detect player entry
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AFoxEnemySpawnVolume::LoadActor_Implementation()
{
	// Check if the player has already reached this spawn volume in a previous session
	if (bReached)
	{
		// Destroy this spawn volume actor since it has already been triggered and is no longer needed
		Destroy();
	}
}

// Called when the game starts or when spawned
void AFoxEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind the OnBoxOverlap callback to the box component's overlap event to trigger enemy spawning when the player enters
	Box->OnComponentBeginOverlap.AddDynamic(this, &AFoxEnemySpawnVolume::OnBoxOverlap);
}

void AFoxEnemySpawnVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Exit early if the overlapping actor is not the player to prevent non-player actors from triggering enemy spawns
	if (!OtherActor->Implements<UPlayerInterface>()) return;

	// Mark this spawn volume as reached so it won't trigger again when the game is saved and loaded
	bReached = true;
	
	// Iterate through all configured spawn points to activate enemy spawning at each location
	for (AFoxEnemySpawnPoint* Point : SpawnPoints)
	{
		// Verify the spawn point reference is valid before attempting to spawn to prevent null pointer access
		if (IsValid(Point))
		{
			// Trigger the spawn point to create and initialize an enemy actor at its location
			Point->SpawnEnemy();
		}
	}
	// Disable collision on the box component to prevent this volume from triggering again after enemies have spawned
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
