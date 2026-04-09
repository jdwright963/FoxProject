// Copyright TryingToMakeGames


#include "Actor/PointCollection.h"

#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APointCollection::APointCollection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Create the first point component that will serve as the root
	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	
	// Add the point to the ImmutablePts array for iteration
	ImmutablePts.Add(Pt_0);
	
	// Set Pt_0 as the actor's root component.
	// Why Pt_0 is the root:
	// - Every Actor in Unreal Engine requires a root component that defines the Actor's transform (location, rotation, scale) in the world.
	// - By making Pt_0 the root, it becomes the "anchor" point that determines where this PointCollection Actor is positioned.
	// 
	// How attachment hierarchy works:
	// - When other points (Pt_1 through Pt_10) are attached to Pt_0 using SetupAttachment(GetRootComponent()),
	//   they become children in the scene component hierarchy.
	// - Child components inherit their parent's transform, meaning:
	//   * If you move/rotate/scale the Actor (which moves Pt_0), all attached points move/rotate/scale with it automatically.
	//   * Each child's transform is stored relative to its parent (Pt_0), maintaining their spatial relationships.
	// - This makes it easy to reposition the entire point collection by just moving the Actor,
	//   rather than manually updating each individual point's world location.
	// - In the editor, you can position each point relative to Pt_0, and this relationship is preserved at runtime.
	SetRootComponent(Pt_0);
	
	// Create the second point component
	Pt_1 = CreateDefaultSubobject<USceneComponent>("Pt_1");
	
	// Add the point to the ImmutablePts array for iteration
	ImmutablePts.Add(Pt_1);
	
	// Attach this point to the root component (Pt_0)
	Pt_1->SetupAttachment(GetRootComponent());

	Pt_2 = CreateDefaultSubobject<USceneComponent>("Pt_2");
	ImmutablePts.Add(Pt_2);
	Pt_2->SetupAttachment(GetRootComponent());

	Pt_3 = CreateDefaultSubobject<USceneComponent>("Pt_3");
	ImmutablePts.Add(Pt_3);
	Pt_3->SetupAttachment(GetRootComponent());
	
	Pt_4 = CreateDefaultSubobject<USceneComponent>("Pt_4");
	ImmutablePts.Add(Pt_4);
	Pt_4->SetupAttachment(GetRootComponent());

	Pt_5 = CreateDefaultSubobject<USceneComponent>("Pt_5");
	ImmutablePts.Add(Pt_5);
	Pt_5->SetupAttachment(GetRootComponent());
	
	Pt_6 = CreateDefaultSubobject<USceneComponent>("Pt_6");
	ImmutablePts.Add(Pt_6);
	Pt_6->SetupAttachment(GetRootComponent());

	Pt_7 = CreateDefaultSubobject<USceneComponent>("Pt_7");
	ImmutablePts.Add(Pt_7);
	Pt_7->SetupAttachment(GetRootComponent());

	Pt_8 = CreateDefaultSubobject<USceneComponent>("Pt_8");
	ImmutablePts.Add(Pt_8);
	Pt_8->SetupAttachment(GetRootComponent());

	Pt_9 = CreateDefaultSubobject<USceneComponent>("Pt_9");
	ImmutablePts.Add(Pt_9);
	Pt_9->SetupAttachment(GetRootComponent());

	Pt_10 = CreateDefaultSubobject<USceneComponent>("Pt_10");
	ImmutablePts.Add(Pt_10);
	Pt_10->SetupAttachment(GetRootComponent());
}

TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,
	float YawOverride)
{
	// Runtime assertion that validates we have enough points available.
	// checkf is a debug macro that crashes the program if the condition is false, and displays the error message.
	// This ensures NumPoints (requested number of points) doesn't exceed the actual number of points in ImmutablePts (11 points: Pt_0 through Pt_10).
	// For example, if NumPoints=15 but we only have 11 points, this will crash with a helpful error message rather than silently failing.
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("Attempted to access ImmutablePts out of bounds."));

	// Create a new empty array to store the points we'll return.
	// This will be populated with up to NumPoints scene components that have been adjusted to ground positions.
	// We use a copy instead of modifying ImmutablePts directly to allow multiple calls with different NumPoints values.
	TArray<USceneComponent*> ArrayCopy;

	// Iterate through each point in the ImmutablePts array (Pt_0 through Pt_10).
	// For each iteration, 'Pt' will reference one USceneComponent* from the array.
	// We use a range-based for loop for clean, readable iteration without needing index variables.
	for (USceneComponent* Pt : ImmutablePts)
	{
		// Early exit condition: once we've collected the requested number of points, return immediately.
		// This prevents processing more points than needed. For example, if NumPoints=5, we only process
		// the first 5 points (Pt_0 through Pt_4) and skip the rest (Pt_5 through Pt_10).
		if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;
		
		// Skip rotation for Pt_0 since it serves as the anchor/pivot point.
		// Only points Pt_1 through Pt_10 need to be rotated around Pt_0.
		// If we rotated Pt_0, we'd lose our reference origin for the rotation operation.
		if (Pt != Pt_0)
		{
			// Calculate the vector from Pt_0 (origin/pivot) to the current point.
			// This gives us the direction and distance from the center (Pt_0) to this point.
			// GetComponentLocation() returns world space coordinates, so subtraction gives us the offset vector.
			// For example, if Pt_0 is at (100, 100, 0) and Pt is at (150, 100, 0), ToPoint would be (50, 0, 0).
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();

			// Rotate the offset vector around the Z-axis (vertical/up axis) by YawOverride degrees.
			// RotateAngleAxis performs rotation around an arbitrary axis (FVector::UpVector = (0, 0, 1)).
			// YawOverride allows the caller to specify a custom rotation angle, enabling dynamic point pattern rotation.
			// This rotates the entire point collection pattern horizontally while maintaining distances from Pt_0.
			// Example: if ToPoint is (50, 0, 0) and YawOverride is 90°, result would be approximately (0, 50, 0).
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);

			// Apply the rotated offset back to Pt_0's location to get the new world position.
			// We add the rotated ToPoint vector to Pt_0's current world location to calculate where this point should now be.
			// This maintains the original distance from Pt_0 while applying the rotation transformation.
			// SetWorldLocation moves the component to absolute world coordinates (not relative/local space).
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}
		
		// Create a point 500 units above the current point's location for the line trace start position.
		// We keep the X and Y coordinates the same (Pt->GetComponentLocation().X/Y) to trace straight down vertically,
		// and add 500.f to the Z coordinate to raise it upward. This should ensure the trace starts well above any ground
		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);

		// Create a point 500 units below the current point's location for the line trace end position.
		// Similar to RaisedLocation, we maintain X/Y coordinates but subtract 500.f from Z to lower it downward.
		// This creates a 1000-unit vertical trace range (500 up + 500 down = 1000 total) centered on the original point position.
		// This range should be sufficient to find ground in most level designs while keeping the trace efficient.
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);

		// Declare a FHitResult struct to store collision information from the line trace.
		// After the trace executes, this will contain data about what was hit (if anything), including:
		// - ImpactPoint: the exact 3D coordinates where the trace hit geometry (used for ground Z position)
		// - ImpactNormal: the surface normal vector at the hit location (used to align rotation to ground slope)
		// - bBlockingHit: whether the trace hit something that blocks (true if ground was found)
		FHitResult HitResult;

		// Create an array to store actors that should be ignored during the line trace.
		// We'll populate this with live players to prevent the trace from hitting player characters,
		// ensuring we only detect actual ground/environment geometry and not characters standing on it.
		TArray<AActor*> IgnoreActors;

		// Call a custom library function to find all live players within a 1500-unit radius of this actor.
		// Parameters breakdown:
		// - this: the calling actor (APointCollection instance) used as wold context for the search to specify in which world to search
		// - IgnoreActors: output parameter that this function will fill with actors to ignore (live players in this case)
		// - TArray<AActor*>(): empty array for additional actors to include (empty since it is not needed here)
		// - 1500.f: search radius in unreal units (covers a large area to catch nearby players)
		// - GetActorLocation(): center point of the sphere search (this PointCollection actor's location)
		// This ensures players won't interfere with ground detection.
		UFoxAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		// Create collision query parameters to configure how the line trace behaves.
		// FCollisionQueryParams allows us to customize trace behavior, such as which actors to ignore.
		FCollisionQueryParams QueryParams;

		// Add all actors from the IgnoreActors array to the query parameters.
		// This tells the physics system to skip collision testing against these actors during the trace.
		// Since IgnoreActors contains live players, the trace will pass through them and only hit static geometry.
		QueryParams.AddIgnoredActors(IgnoreActors);

		// Perform a line trace using the "BlockAll" collision profile.
		// This casts a ray from RaisedLocation (500 units up) to LoweredLocation (500 units down),
		// searching for the first blocking object along that vertical line. Parameters:
		// - HitResult: output struct filled with collision data if something is hit
		// - RaisedLocation: trace start point (above the point)
		// - LoweredLocation: trace end point (below the point)
		// - FName("BlockAll"): collision profile that responds to all blocking geometry (walls, floors, etc.)
		// - QueryParams: additional settings including the actors to ignore
		// Returns true if blocking geometry was hit, false otherwise. Result stored in HitResult.
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		// Create a new location vector using the point's current X/Y coordinates but replacing Z with the trace hit height.
		// This keeps the point's horizontal position (X, Y) unchanged while snapping its vertical position (Z)
		// to match the ground surface exactly. HitResult.ImpactPoint.Z contains the Z coordinate where the trace
		// hit the ground geometry, giving us the precise ground height at this point's X/Y location.
		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);

		// Move the point component to the adjusted location, effectively snapping it to the ground surface.
		// SetWorldLocation updates the component's position in world space (absolute coordinates).
		// Now the point sits exactly on the ground at the correct height, ready for spawning objects or effects.
		Pt->SetWorldLocation(AdjustedLocation);

		// Rotate the point to align with the ground surface normal (perpendicular to the surface).
		// UKismetMathLibrary::MakeRotFromZ creates a rotation where the Z-axis (up vector) points in the direction
		// of HitResult.ImpactNormal (the surface normal at the hit point). This makes the point "stand upright"
		// relative to the ground slope, so spawned objects will align naturally with sloped or angled terrain
		// rather than always being perfectly vertical. Useful for placing effects/actors on stairs, hills, etc.
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));
		
		// Add the ground-adjusted point to the output array.
		// After all transformations (rotation around Pt_0, ground snapping, and surface alignment),
		// this point is now ready to be used for spawning objects/effects at ground level.
		// The array accumulates points until we reach NumPoints or run out of ImmutablePts.
		ArrayCopy.Add(Pt);
	}

	// Return the array of ground-snapped points to the caller.
	// The caller can now use these points as spawn locations, target positions, etc.
	return ArrayCopy;
}

// Called when the game starts or when spawned
void APointCollection::BeginPlay()
{
	Super::BeginPlay();
}


