// Copyright TryingToMakeGames


#include "Actor/MagicCircle.h"

#include "Components/DecalComponent.h"

// Sets default values
AMagicCircle::AMagicCircle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates the decal component for the magic circle visual and attaches it to the root component
	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	MagicCircleDecal->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AMagicCircle::BeginPlay()
{
	// Calls the parent BeginPlay function
	Super::BeginPlay();
	
}

// Called every frame
void AMagicCircle::Tick(float DeltaTime)
{
	// Calls the parent Tick function
	Super::Tick(DeltaTime);

}

