// Copyright TryingToMakeGames


#include "Actor/FoxEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFoxEffectActor::AFoxEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AFoxEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Accumulate elapsed time to track progress through the sinusoidal movement cycle
	RunningTime += DeltaTime;
	
	// Calculate the period (duration of one complete cycle) of the sine wave used for vertical movement
	const float SinePeriod = 2 * PI / SinePeriodConstant;
	
	// Check if the running time has exceeded one complete sine wave period to prevent time overflow
	if (RunningTime > SinePeriod)
	{
		// Reset the running time to zero to start the next sine wave cycle from the beginning
		RunningTime = 0.f;
	}
	// Update the actor's rotation and vertical sinusoidal position based on the current time and delta time
	ItemMovement(DeltaTime);
}

void AFoxEffectActor::ItemMovement(float DeltaTime)
{
	// Check if rotation is enabled for this effect actor
	if (bRotates)
	{
		// Calculate the incremental rotation (yaw only) to apply this frame based on the rotation rate and delta time
		const FRotator DeltaRotation(0.f, DeltaTime * RotationRate, 0.f);
		
		// Compose the new rotation by combining the current calculated rotation with the delta rotation
		CalculatedRotation = UKismetMathLibrary::ComposeRotators(CalculatedRotation, DeltaRotation);
	}
	// Check if sinusoidal (up and down) movement is enabled for this effect actor
	if (bSinusoidalMovement)
	{
		// Calculate the current vertical offset using a sine wave based on elapsed time, amplitude, and period constant
		const float Sine = SineAmplitude * FMath::Sin(RunningTime * SinePeriodConstant);
		
		// Update the calculated location by adding the sine wave offset to the Z component of the initial location
		CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);
	}
}

// Called when the game starts or when spawned
void AFoxEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Store the actor's initial world location to use as the base position for sinusoidal movement calculations
	InitialLocation = GetActorLocation();
	
	// Initialize the calculated location to the starting position before any movement transformations are applied
	CalculatedLocation = InitialLocation;
	
	// Store the actor's initial world rotation to use as the base rotation for incremental rotation calculations
	CalculatedRotation = GetActorRotation();
}

void AFoxEffectActor::StartSinusoidalMovement()
{
	// Enable sinusoidal (up and down) movement for this effect actor
	bSinusoidalMovement = true;
	
	// Store the actor's current world location to use as the base position for sinusoidal movement calculations
	InitialLocation = GetActorLocation();
	
	// Initialize the calculated location to the starting position before any movement transformations are applied
	CalculatedLocation = InitialLocation;
}

void AFoxEffectActor::StartRotation()
{
	// Enable rotation for this effect actor
	bRotates = true;

	// Store the actor's current world rotation to use as the base rotation for incremental rotation calculations
	CalculatedRotation = GetActorRotation();
}

void AFoxEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// If the target actor is an enemy (has the actor tag "Enemy") and effects should not be applied to enemies
	// (bApplyEffectsToEnemies is false), return early
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	/*
	 * Retrieve the Ability System Component (ASC) from the target actor.
	 * If the target doesn't have an ASC, exit early as we cannot apply effects.
	 * Verify that a valid GameplayEffect class was provided (crashes in debug if null).
	 */
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	check(GameplayEffectClass);
	
	/*
	 * Create a context handle to store metadata about the gameplay effect (instigator, source, etc.).
	 * Add this actor as the source object so the effect knows where it originated from.
	 * Create an outgoing effect specification from the GameplayEffect class with level 1.0 and the context.
	 */
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	
	/*
	 * Apply the gameplay effect specification to the target's ability system component.
	 * 
	 * Breaking down this line:
	 * - TargetASC->ApplyGameplayEffectSpecToSelf(): This method applies a gameplay effect to the owner of the ASC.
	 * - EffectSpecHandle.Data: This is a TSharedPtr<FGameplayEffectSpec> that holds the actual effect specification data.
	 * - .Get(): Retrieves the raw pointer (FGameplayEffectSpec*) from the shared pointer.
	 * - *: Dereferences the raw pointer to pass the actual FGameplayEffectSpec object by reference.
	 * 
	 * The method expects a const FGameplayEffectSpec& parameter, so we must dereference the pointer.
	 * When we pass *EffectSpecHandle.Data.Get() as an argument, the method's parameter signature (const FGameplayEffectSpec&)
	 * automatically binds the dereferenced object as a const reference rather than a regular reference. This happens because
	 * the function signature explicitly declares the parameter as const, making it read-only and preventing modifications.
	 * This applies the configured effect (with all its modifiers, duration, context) to the target actor.
	 */
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	
	/*
	 * Check if the applied gameplay effect has an infinite duration policy.
	 * 
	 * Breaking down this line:
	 * - EffectSpecHandle: This is an FGameplayEffectSpecHandle, a wrapper struct that contains effect specification data.
	 * - .Data: This is a TSharedPtr<FGameplayEffectSpec> member variable that holds the actual effect specification.
	 * - .Get(): Retrieves the raw pointer (FGameplayEffectSpec*) from the shared pointer.
	 * - ->Def: This is a TSharedPtr<FGameplayEffectSpec> member in FGameplayEffectSpec that references the effect definition.
	 * - .Get(): Retrieves the raw pointer (UGameplayEffect*) from the definition shared pointer.
	 * - ->DurationPolicy: This is an EGameplayEffectDurationType enum property that defines how long the effect lasts.
	 * - == EGameplayEffectDurationType::Infinite: Compares the duration policy to check if it's set to Infinite.
	 * 
	 * This check is important because infinite effects need to be tracked and potentially removed manually later.
	 * Instant effects apply once and disappear, duration effects expire automatically, but infinite effects persist
	 * until explicitly removed (typically on end overlap based on InfiniteEffectRemovalPolicy).
	 */
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		/*
		 * If the duration type is infinite and the removal policy is remove on end overlap, then we add the to map 
		 * member variable a key value pair where the active effect handle is the key and the target ASC is the value
		*/ 
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}
	
	// Only destroy the effect actor if it is not an infinite effect type
	if (!bIsInfinite)
	{
		Destroy();
	}
}

void AFoxEffectActor::OnOverlap(AActor* TargetActor)
{
	// If the target actor is an enemy (has the actor tag "Enemy") and effects should not be applied to enemies
	// (bApplyEffectsToEnemies is false), return early
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);	
	}
}

void AFoxEffectActor::OnEndOverlap(AActor* TargetActor)
{
	// If the target actor is an enemy (has the actor tag "Enemy") and effects should not be applied to enemies
	// (bApplyEffectsToEnemies is false), return early
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);	
	}
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		// Get the ASC using the target actor passed into this function, and return early if it is not valid
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;
		
		/*
		 * Create an array to store the key value pairs we wish to remove from the map until the loop is exited, 
		 * because removing the items during the loop can cause a crash
		*/
		TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		
		/*
		 * Iterate through the ActiveEffectHandles map to find and remove all infinite effects that belong to the target actor.
		 * 
		 * Breaking down this loop:
		 * - TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*>: This is the type of each key-value pair in the map.
		 *   The first element (Key) is an FActiveGameplayEffectHandle identifying the active effect.
		 *   The second element (Value) is a pointer to the UAbilitySystemComponent that owns this effect.
		 * - HandlePair: A copy of each map entry as we iterate (copying is safe here since we're not modifying the loop variable).
		 * - ActiveEffectHandles: The TMap member variable storing all infinite effects this actor (or all actors? other effect actors
		 * have their own map but i think each map in an effect actor is for all actors in the world) has applied.
		 * 
		 * Inside the loop:
		 * - We compare TargetASC (the ASC of the actor that just ended overlap) with HandlePair.Value (the ASC stored in the map).
		 * - If they match, this means the effect belongs to the actor ending overlap, so we need to remove it.
		 * - TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1): Removes the actual gameplay effect from the target's ASC.
		 * The '1' argument specifies that only 1 stack will be removed on end overlap. We do this because each instance
		 * of this effect actor has its own map and it therefore responsible for removing itself on end overlap.
		 * - HandlesToRemove.Add(HandlePair.Key): Tracks which handles need to be removed from our map after the loop.
		 * 
		 * We cannot remove items from ActiveEffectHandles during iteration as it would invalidate the iterator and cause a crash.
		 * Instead, we collect handles to remove and clean up the map in a separate loop afterwards.
		 */
		for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		
		/*
		 * Clean up the ActiveEffectHandles map by removing all entries that were marked for removal.
		 * 
		 * Now that we've exited the iteration loop, it's safe to modify the ActiveEffectHandles map.
		 * We iterate through the HandlesToRemove array and use FindAndRemoveChecked() to remove each handle.
		 * FindAndRemoveChecked() will find the key in the map, remove it, and assert if the key doesn't exist
		 * (which shouldn't happen since we just added these keys in the previous loop).
		 */
		for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}



