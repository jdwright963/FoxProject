// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCollection.generated.h"

UCLASS()
class FOX_API APointCollection : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointCollection();
	
	// Returns an array that contains all the points adjusted to ground level.
	// Performs a line trace downward from each point in the ImmutablePts array to find the ground surface,
	// then positions the point at that ground level.
	// If YawOverride is provided (non-zero), all points are rotated around Pt_0 on the Z-axis by that angle.
	// 
	// @param GroundLocation - The spot on the ground the player is targeting with the cursor.
	// @param NumPoints - The number of points from ImmutablePts array to process and include in the returned array
	// @param YawOverride - Optional rotation angle in degrees to rotate all points around Pt_0 on the Z-axis (default: 0.f means no rotation)
	UFUNCTION(BlueprintPure)
	TArray<USceneComponent*> GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride = 0.f);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Array for storing all point components (Pt_0 through Pt_10) for easy iteration and access.
	// Populated during construction of this class.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<USceneComponent*> ImmutablePts;

	// Individual scene component points that can be positioned in the editor to define spawn/target locations.
	// Pt_0 serves as the root component. Pt_1 through Pt_10 follow the same pattern and are attached to Pt_0.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_3;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_4;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_5;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_6;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_7;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_8;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_9;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_10;

};
