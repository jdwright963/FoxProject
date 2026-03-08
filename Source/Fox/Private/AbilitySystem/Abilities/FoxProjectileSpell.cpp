// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/FoxProjectile.h"
#include "Interaction/CombatInterface.h"

void UFoxProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	/* 
	 * Spawn the projectile on server not on client and then the server can replicate it down to clients 
	 * Checking for authority (which will only be true on the server) using the function from the GameplayAbility class 
	 * which requires a FGameplayAbilityActivationInfo pointer which is a input parameter to the function definition we 
	 * are inside of, but it is not a pointer. So we can use the address of operator to pass that parameter the address 
	*/
	// const bool bIsServer = HasAuthority(&ActivationInfo);
}

void UFoxProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	/* 
	 * We only want to spawn the projectile on server not on client and then the server can replicate it down to clients 
	 * GetAvatarActorFromActorInfo returns the physical actor that is executing this ability. It may be null. Then, we 
	 * check if this actor has network authority (which will only be true on the server)
	*/
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	
	// If we are not on the server, return early
	if (!bIsServer) return;
	
	/*
	 * Retrieve the combat socket location (typically the tip of the weapon) from the avatar actor.
	 *
	 * ICombatInterface::Execute_GetCombatSocketLocation is an auto-generated static helper function created
	 * by Unreal's interface system when GetCombatSocketLocation was marked as a BlueprintNativeEvent in the
	 * CombatInterface. This Execute_ function provides safe cross-language invocation that works whether
	 * GetCombatSocketLocation is implemented in C++ or Blueprint.
	 *
	 * The Execute_ function internally checks if the target actor (GetAvatarActorFromActorInfo() in this case)
	 * has a Blueprint implementation of GetCombatSocketLocation. If it does, the Blueprint version is called.
	 * If not, it falls back to the C++ _Implementation version (GetCombatSocketLocation_Implementation).
	 *
	 * This pattern allows designers to override combat socket logic in Blueprint classes while still having
	 * a C++ default implementation, providing maximum flexibility without manual type checking or casting.
	 * It's the recommended way to call BlueprintNativeEvent functions when you're unsure whether the
	 * implementation is in C++ or Blueprint.
	 *
	 * The second parameter (MontageTag) is a gameplay tag that identifies which socket to retrieve. In this
	 * case, we pass in the SocketTag that is an input parameter of the current function. This allows the same
	 * interface function to return different socket locations based on the context (weapon attacks, shield
	 * blocks, spell casting points, etc.) by using different tags for different combat scenarios.
	*/
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		SocketTag
		);
	
	// 'ProjectileTargetLocation - SocketLocation' gives us the vector from the socket location to the 
	// ProjectileTargetLocation. 'Rotation()' Returns the TRotator<T> (an alias for FRotator) orientation 
	// corresponding to the direction in which the vector points. 
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	
	// If the pitch override input parameter (bOverridePitch) is enabled, modify the rotation pitch
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}
	
	// Variable for the world location where the projectile will be spawned
	FTransform SpawnTransform;
	
	// Set the location of the spawn transform to the socket location (the tip of the character's weapon)
	SpawnTransform.SetLocation(SocketLocation);
	
	/*
	 * Set the projectile rotation
	 * Sets the rotation of the spawn transform using a Quaternion representation of our Rotation variable.
	 * A Quaternion is a four-component math object (X, Y, Z, W) that represents a 3D orientation. While 
	 * humans prefer Rotators (Pitch, Yaw, Roll), computers use Quaternions to avoid "Gimbal Lock."
	 *
	 * Gimbal Lock is a "math glitch" caused by the way Rotators calculate movement in a specific order 
	 * (like Yaw, then Pitch, then Roll). Imagine an airplane pointing level: Yaw turns it left/right, 
	 * and Roll spins it like a drill. However, if you Pitch the nose 90 degrees to point straight up 
	 * at the sky, your "Roll" axis and "Yaw" axis now point in the exact same direction. 
	 *
	 * When these two axes align, they effectively "lock" together. Rotating your Yaw and rotating 
	 * your Roll will now perform the exact same spinning motion. You have "lost a degree of freedom" 
	 * because there is no longer a unique way to tilt the plane to the side without first moving 
	 * the nose away from that 90-degree angle. This causes the jerky "flipping" seen in some games.
	 *
	 * Quaternions solve this because they do not calculate rotations one axis at a time. Instead of 
	 * saying "Rotate X, then Y, then Z," a Quaternion treats the entire rotation as a single "hop" 
	 * to the final orientation. Because it doesn't use a sequence of nested steps, its axes can 
	 * never align or lock, ensuring the projectile always points exactly where the math expects.
	*/
	SpawnTransform.SetRotation(Rotation.Quaternion());

	/*
	 * Spawn the projectile using deferred spawning, a two-step process that allows property configuration before 
	 * the actor fully initializes and begins play. SpawnActorDeferred() creates the actor instance and calls its 
	 * constructor, but delays calling BeginPlay() until FinishSpawning() is called, giving us a window to set up 
	 * critical properties that must be configured before the actor starts ticking and interacting with the world.
	 *
	 * Deferred spawning is used here because we need to set multiple critical properties on the projectile 
	 * before it begins play and starts interacting with the world:
	 * - DamageEffectParams must be configured before OnSphereOverlap can be triggered by collisions
	 * - Homing target configuration (HomingTargetComponent) must be set before the ProjectileMovementComponent 
	 *   begins calculating trajectory adjustments in its Tick function
	 * - Homing parameters (HomingAccelerationMagnitude, bIsHomingProjectile) need to be initialized before 
	 *   movement calculations start
	 *
	 * If we used normal spawning (SpawnActor), BeginPlay would be called immediately after construction, 
	 * causing the ProjectileMovementComponent to start ticking and the collision sphere to start detecting 
	 * overlaps before we could configure these essential properties. With deferred spawning, we can set all 
	 * properties while the actor is in a "pre-initialized" state, then call FinishSpawning to complete the 
	 * spawn process and trigger BeginPlay with everything properly configured.
	 *
	 * TEMPLATE PARAMETER:
	 * AFoxProjectile: The template parameter specifies the C++ class type that SpawnActorDeferred should return,
	 *    enabling compile-time type safety and eliminating the need for runtime casting. By passing AFoxProjectile
	 *    as the template argument, the function's return type is automatically AFoxProjectile* instead of the
	 *    generic AActor*, allowing us to directly access projectile-specific properties (DamageEffectParams,
	 *    ProjectileMovement, HomingTargetSceneComponent) without manual Cast<AFoxProjectile>() calls. This template
	 *    parameter restricts spawning to AFoxProjectile or its derived classes (like BP_FireBolt), preventing
	 *    compile-time errors if someone accidentally tries to spawn an incompatible actor type. The template system
	 *    works with the first function parameter (ProjectileClass) to ensure the UClass being instantiated is
	 *    compatible with the template type, providing both runtime flexibility (designers can change which Blueprint
	 *    to spawn) and compile-time safety (the code enforces the spawned actor will have projectile functionality).
	 *
	 * PARAMETERS:
	 * 1. ProjectileClass (TSubclassOf<AFoxProjectile>): The UClass type to instantiate, specifying which Blueprint
	 *    or C++ class to spawn at runtime. We pass ProjectileClass, a blueprint-configurable member variable
	 *    (inherited from UFoxProjectileSpell) that allows designers to assign different projectile types (fire bolts,
	 *    ice shards, lightning orbs, etc.) in the ability's Blueprint without modifying C++ code. TSubclassOf is a
	 *    template wrapper that restricts the variable to only accept AFoxProjectile-derived classes, preventing
	 *    designers from accidentally assigning incompatible actor types (like ACharacter or APickup) in the Blueprint
	 *    editor. At runtime, this UClass pointer is used by the spawn system to determine which constructor to call
	 *    and which default properties to copy, enabling the same C++ ability code to spawn completely different
	 *    projectile visuals, speeds, and behaviors just by changing the Blueprint asset reference.
	 *
	 * 2. SpawnTransform (const FTransform&): The world-space transform (location, rotation, scale) where the actor 
	 *    will be spawned. We pass SpawnTransform, a local variable constructed earlier in the loop that positions 
	 *    the projectile at SocketLocation (typically the weapon tip) with rotation Rot (one of the spread directions 
	 *    calculated by EvenlySpacedRotators). This transform defines the projectile's initial position and facing 
	 *    direction in the game world. Each projectile in the multi-shot pattern gets a unique transform with the 
	 *    same spawn location but different rotation to create the spreading fan effect.
	 *
	 * 3. Owner (AActor*): The actor that logically "owns" this spawned projectile for gameplay purposes, used by 
	 *    the engine for network replication ownership, relevancy calculations, and some gameplay queries. We pass 
	 *    GetOwningActorFromActorInfo(), which retrieves the PlayerController or AIController that activated this 
	 *    ability. Setting this as the owner ensures the projectile is replicated to clients that can see its owner, 
	 *    and provides proper attribution for networked gameplay (kill credits, team affiliation, etc.). This is 
	 *    NOT the same as the Instigator parameter. Owner is typically the controlling actor, while Instigator is 
	 *    the physical pawn performing the action.
	 *
	 * 4. Instigator (APawn*): The pawn that physically caused this projectile to be spawned, used for damage 
	 *    attribution, friendly fire checks, and gameplay logic that needs to know which character performed an 
	 *    action. We pass Cast<APawn>(GetOwningActorFromActorInfo()), which attempts to cast the ability's owning 
	 *    actor to APawn. In most cases this will be the player's character or an AI-controlled pawn that cast the 
	 *    spell. The cast ensures type safety since Instigator specifically requires an APawn pointer. If the 
	 *    owning actor isn't a pawn (unlikely for character abilities, but possible in edge cases), the cast returns 
	 *    nullptr and the projectile spawns without an Instigator. This Instigator reference gets stored in the 
	 *    projectile's base AActor properties and is used later when applying damage to determine who should receive 
	 *    credit for kills, experience points, or gameplay events triggered by the projectile's impact.
	 *
	 * 5. SpawnCollisionHandlingMethod (ESpawnActorCollisionHandlingMethod): An enum that controls how the spawn 
	 *    system handles cases where the spawn location is blocked by existing geometry or actors. We pass 
	 *    ESpawnActorCollisionHandlingMethod::AlwaysSpawn, which forces the projectile to spawn at the specified 
	 *    transform regardless of collision state. This prevents spawn failures when the weapon socket clips into 
	 *    walls during tight-quarters combat or when the player is standing near geometry. Alternative options like 
	 *    AdjustIfPossibleButDontSpawnIfColliding would cause projectiles to fail to spawn in cramped spaces, 
	 *    creating frustrating situations where the ability consumes resources but produces no projectile. AlwaysSpawn 
	 *    ensures reliable ability execution, with the minor trade-off that projectiles might spawn partially inside 
	 *    geometry and immediately collide, but this is preferable to abilities that randomly fail to work when needed.
	*/
	AFoxProjectile* Projectile = GetWorld()->SpawnActorDeferred<AFoxProjectile>(
		ProjectileClass, 
		SpawnTransform, 
		GetOwningActorFromActorInfo(), 
		Cast<APawn>(GetOwningActorFromActorInfo()), 
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
	
	/*
	 * Configure the projectile's damage parameters before it begins play.
	 *
	 * MakeDamageEffectParamsFromClassDefaults() is a helper function (defined in the parent class
	 * UFoxDamageGameplayAbility) that constructs an FDamageEffectParams struct populated with default
	 * values configured for this ability class. This struct contains all the information needed to apply
	 * damage when the projectile hits a target, including:
	 * - The damage value or calculation
	 * - Damage type
	 * - Source and target references
	 * - Any gameplay tags
	 *
	 * We must set this property during deferred spawning (before FinishSpawning is called) because the
	 * projectile's OnSphereOverlap function will need access to these damage parameters as soon as the
	 * projectile begins play and starts detecting collisions. Setting it here ensures the projectile is
	 * fully configured with damage information before it can interact with the world.
	*/
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
	
	// Finish spawning the projectile at the specified transform
	Projectile->FinishSpawning(SpawnTransform);
}
