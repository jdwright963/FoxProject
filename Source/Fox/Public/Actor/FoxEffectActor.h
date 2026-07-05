// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "FoxEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EEffectApplicationPolicy : uint8
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

// This enum is only applicable to infinite gameplay effects, because they are the only gameplay effects
// that need to be removed manually. In other cases a duration gameplay effect could also be remove manually on 
// end overlap
UENUM(BlueprintType)
enum class EEffectRemovalPolicy : uint8
{
	RemoveOnEndOverlap,
	DoNotRemove
};

UCLASS()
class FOX_API AFoxEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFoxEffectActor();
	
	// Called every frame to update actor state, handling rotation and sinusoidal movement animations based on configuration
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// The calculated location for this actor updated each frame based on sinusoidal movement settings
	UPROPERTY(BlueprintReadWrite)
	FVector CalculatedLocation;

	// The calculated rotation for this actor updated each frame based on rotation settings
	UPROPERTY(BlueprintReadWrite)
	FRotator CalculatedRotation;

	// Variable to control whether the actor should continuously rotate around its Z-axis during gameplay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	bool bRotates = false;

	// The speed at which the actor rotates around its Z-axis, measured in degrees per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float RotationRate = 45.f;

	// Variable to control whether the actor should perform sinusoidal (wave-like) vertical movement animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	bool bSinusoidalMovement = false;

	// Function to enable sinusoidal movement animation for this actor, causing it to move up and down in a wave pattern
	UFUNCTION(BlueprintCallable)
	void StartSinusoidalMovement();

	// Function to enable continuous rotation animation for this actor around its Z-axis
	UFUNCTION(BlueprintCallable)
	void StartRotation();
	
	// The amplitude (maximum vertical displacement) of the sinusoidal wave movement in Unreal units
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SineAmplitude = 1.f;

	// The period constant that controls the frequency/speed of the sinusoidal oscillation (higher values = faster movement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SinePeriodConstant = 1.f; 

	// The initial location of the actor stored in BeginPlay, used as the base point for calculating sinusoidal movement offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	FVector InitialLocation;
	
	// Applies a gameplay effect to the target actor's ability system component
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);
	
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	// Variable to control whether actor should be destroyed when the effect is applied
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectApplication = false;

	// Variable to control whether effects should be applied to actors with the "Enemy" actor tag
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bApplyEffectsToEnemies = false;

	// The gameplay effect class that will be applied instantly when triggered
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// The gameplay effect class that will be applied for a duration when triggered
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	// The gameplay effect class that will be applied when triggered for an infinite duration until removed manually
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;
	
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Applied Effects")
	float ActorLevel = 1.f;
	
private:
	
	// Tracks the total elapsed time in seconds since BeginPlay, used to calculate the sinusoidal wave position for vertical movement animation
	float RunningTime = 0.f;

	// Helper function called each frame to update the actor's rotation and sinusoidal movement based on configuration settings
	void ItemMovement(float DeltaTime);
};
