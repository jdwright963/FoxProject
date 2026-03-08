// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxSummonAbility.h"

TArray<FVector> UFoxSummonAbility::GetSpawnLocations()
{
	// Get the forward direction vector of the avatar actor performing the summon
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	
	// Get the world location of the avatar actor performing the summon
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	
	// Calculate the angle between each minion spawn point by dividing the total spread angle by the number of minions
	const float DeltaSpread = SpawnSpread / NumMinions;

	// Rotate the Forward vector variable counter-clockwise (negative angle) by half of the total spawn spread angle around the up (Z) axis.
	// This creates the leftmost edge of the spawn spread arc. By starting from the left edge and incrementally rotating
	// clockwise by DeltaSpread for each minion, we can evenly distribute minions across the full SpawnSpread angle.
	// For example, with SpawnSpread = 90 degrees, this rotates Forward by -45 degrees to start from the left edge.
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	
	// Array to hold the calculated spawn locations
	TArray<FVector> SpawnLocations;
	
	// Loop that iterates (executes) NumMinions times, calculating spawn locations for each minion
	for (int32 i = 0; i < NumMinions; i++)
	{
		
		// Calculate the direction vector for this minion's spawn by rotating the LeftOfSpread vector clockwise
		// (positive angle) around the up (Z) axis. The rotation angle increases with each iteration (DeltaSpread * i),
		// distributing minions evenly across the spawn spread arc from left to right.
		// For example: i=0 uses LeftOfSpread as-is (leftmost), i=1 rotates by DeltaSpread (one step right), etc.
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);

		
		// Calculate the final spawn location by starting from the avatar's Location and moving along the Direction vector.
		// This uses vector addition: Location + (Direction * Distance), where:
		// 1. FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance) generates a random float between MinSpawnDistance and MaxSpawnDistance
		// 2. Direction * RandomDistance scales the Direction vector (which is normalized/unit length) by the random distance value,
		//    creating a vector that points in the Direction but with length equal to the random distance
		// 3. Location + ScaledDirection performs vector addition, moving from the avatar's position (Location) along the scaled
		//    Direction vector, resulting in a point in world space at the random distance away from the avatar in the calculated direction.
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
		
		// Variable to store the hit result of the line trace operation
		FHitResult Hit;
		
		// Perform a line trace (raycast) straight down to find the ground level beneath the calculated spawn location.
		// This ensures minions spawn on walkable surfaces rather than floating in air or stuck underground.
		// We start the trace 400 units ABOVE the ChosenSpawnLocation because the initially calculated spawn point
		// (based on random distance and direction from the avatar) doesn't account for terrain height variations.
		// The ground could be higher or lower than the avatar's position. By starting above and tracing down through
		// the ChosenSpawnLocation, we ensure the trace can detect ground whether it's on a hill above the avatar,
		// in a valley below, or at the same level. Without starting above, if the ground was higher than ChosenSpawnLocation,
		// the trace would start inside or below the ground and miss detecting the actual surface.
		// LineTraceSingleByChannel parameters:
		// 1. Hit - FHitResult struct that will store information about what the trace hits (if anything)
		// 2. Start Point - ChosenSpawnLocation + FVector(0.f, 0.f, 400.f) creates a point 400 units above the spawn location (positive Z is up)
		// 3. End Point - ChosenSpawnLocation - FVector(0.f, 0.f, 400.f) creates a point 400 units below the spawn location
		//    This creates a vertical line trace 800 units tall (400 up + 400 down) centered on ChosenSpawnLocation
		// 4. ECC_Visibility - The collision channel to trace against; checks for objects that block visibility (typically world geometry/floors)
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		
		// Check if trace hit something (i.e., found a ground level)
		if (Hit.bBlockingHit)
		{
			// If hit, set spawn location to impact point (ground level)
			ChosenSpawnLocation = Hit.ImpactPoint;
		}

		// Add the calculated spawn location to the SpawnLocations array, which will be returned at the end of the function
		// and used to actually spawn the minions at these positions.
		SpawnLocations.Add(ChosenSpawnLocation);
	}
	
	// Return the array of spawn locations, ready for minion spawning
	return SpawnLocations;
}

TSubclassOf<APawn> UFoxSummonAbility::GetRandomMinionClass()
{
	// Generate a random integer index between 0 (first element) and MinionClasses.Num() - 1 (last valid array index).
	// MinionClasses.Num() returns the total number of elements in the array, so subtracting 1 gives us the highest
	// valid index we can use to access array elements (since arrays are zero-indexed). For example, if MinionClasses
	// contains 3 elements, Num() returns 3, but valid indices are 0, 1, and 2, so we use Num() - 1 = 2 as the max.
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);

	// Use the randomly generated Selection index to access and return a minion class from the MinionClasses array.
	return MinionClasses[Selection];
}
