// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicCircle.generated.h"

UCLASS()
class FOX_API AMagicCircle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicCircle();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Decal component that provides the visual representation of the magic circle projected onto surfaces
	// This component has a decal material property that we set in the blueprint this allows us to select the material
	// the decal will project onto surfaces
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDecalComponent> MagicCircleDecal;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	
};
