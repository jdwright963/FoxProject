// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxFireBolt.h"

#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Actor/FoxProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UFoxFireBolt::GetDescription(int32 Level)
{
	/*
	 * Calculate the base damage value for the Fire Bolt ability at the current level by calling GetValueAtLevel() 
	 * on the Damage member variable (an FScalableFloat inherited from UFoxDamageGameplayAbility parent class and 
	 * configured in the Blueprint). FScalableFloat is a struct that stores a base float value and an optional 
	 * reference to a UCurveTable with a row name, allowing designers to define damage progression curves across 
	 * ability levels. When GetValueAtLevel() is called, it evaluates the curve at the specified level parameter 
	 * (if a curve is assigned) or returns the base value (if no curve is configured). The return value is cast to 
	 * int32 to provide a whole number for display purposes, as fractional damage values would look odd in UI 
	 * tooltips (e.g., "25 damage" reads better than "25.3 damage").
	*/
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	
	/*
	 * Calculate the mana cost required to cast the Fire Bolt ability at the current level by calling GetManaCost() 
	 * (inherited from UFoxGameplayAbility parent class) which queries the ability's cost gameplay effect to extract 
	 * the mana modifier's magnitude at the specified level. The cost effect is configured in the Blueprint and 
	 * typically uses a UCurveTable to define how mana cost scales across levels. FMath::Abs() wraps the result to 
	 * ensure the displayed value is always positive, even though gameplay effects internally represent costs as 
	 * negative attribute modifiers (since spending mana reduces the attribute). This conversion prevents the UI from 
	 * showing confusing negative numbers like "-25 mana" and instead displays clean positive values like "25 mana" 
	 * that players intuitively understand as the resource amount they need to spend to activate the ability.
	*/
	const float ManaCost = FMath::Abs(GetManaCost(Level));

	/*
	 * Calculate the cooldown duration in seconds for the Fire Bolt ability at the current level by calling 
	 * GetCooldown() (inherited from UFoxGameplayAbility parent class) which queries the ability's cooldown gameplay 
	 * effect to extract the duration magnitude at the specified level. The cooldown effect is configured in the 
	 * Blueprint and typically uses a UCurveTable or FScalableFloat to define how the cooldown period scales across 
	 * ability levels. The returned float value represents the number of seconds the player must wait after 
	 * casting before the ability becomes available again..
	*/
	const float Cooldown = GetCooldown(Level);

	// Check if this is level 1 to return a description for a single fire bolt
	if (Level == 1)
	{
		/*
		 * Construct and return a formatted description string for the Fire Bolt ability at level 1 using FString::Printf() 
		 * to insert dynamic values into a template string wrapped in the TEXT() macro for proper Unicode/ANSI handling. 
		 * The string contains rich text markup tags that are parsed by Unreal's Rich Text Block UI widget system to apply 
		 * different visual styles to semantically different pieces of information. These markup tags use XML-like syntax 
		 * with opening tags like <TagName> and closing tags </>. Each tag pair identifies a category of 
		 * content that the UI widget will style according to decorator classes or style configurations defined in the 
		 * widget's setup:
		 * 
		 * Each tag such as this `<Title></>` wraps text that will be styled according to the styles set for these tags
		 * in the data table DT_RichTextStyle
		 * 
		 * The \n characters create line breaks in the displayed text for spacing. The %d and %.1f are printf format 
		 * specifiers that get replaced with the actual integer and float values passed as arguments (Level, ManaCost, 
		 * Cooldown, ScaledDamage) after the template string, with %d formatting integers and %.1f formatting floats with one 
		 * decimal place.
		*/
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Launches a bolt of fire, "
			"exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		/*
		 * Construct and return a formatted description string for Fire Bolt at levels 2 and above, where the ability 
		 * launches multiple projectiles simultaneously. This else block executes for any level greater than 1, changing 
		 * the description from singular "a bolt" to plural "bolts" to reflect the multi-projectile mechanic. The string 
		 * construction uses the same FString::Printf() and TEXT() macro pattern with rich text markup tags for UI styling 
		 * as described in the level 1 block above. 
		 * 
		 * The key difference is the projectile count calculation: 
		 * FMath::Min(Level, NumProjectiles) determines how many fire bolts are launched by taking the minimum value 
		 * between the current ability level and the NumProjectiles member variable (defined in the parent 
		 * UFoxProjectileSpell class and configured in the Blueprint). This capping mechanism prevents the projectile 
		 * count from exceeding a designer-specified maximum (for example, if NumProjectiles is set to 5, the ability 
		 * will launch 2 bolts at level 2, 3 bolts at level 3, up to 5 bolts at level 5, and will remain at 5 bolts for 
		 * levels 6, 7, 8, etc.).
		*/
		return FString::Printf(TEXT(
			// Title
			"<Title>FIRE BOLT</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of FireBolts
			"<Default>Launches %d bolts of fire, "
			"exploding on impact and dealing: </>"

			// Damage
			"<Damage>%d</><Default> fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, NumProjectiles),
			ScaledDamage);
	}
}

FString UFoxFireBolt::GetNextLevelDescription(int32 Level)
{
	/*
	 * Calculate the damage value for the Fire Bolt ability at the next level by calling GetValueAtLevel() on the 
	 * Damage FScalableFloat member variable with the Level parameter (typically CurrentLevel + 1). This 
	 * evaluates the damage progression curve configured in the Blueprint at the specified next level, showing players 
	 * what the damage output will be if they invest a skill point to upgrade the ability. The FScalableFloat system 
	 * queries its optional UCurveTable reference at the given level (or returns the base value if no curve is set), 
	 * and the result is cast to int32 for clean whole-number display in the UI tooltip. Displaying the next level 
	 * damage helps players make informed upgrade decisions by seeing the exact damage increase they'll gain, allowing 
	 * them to evaluate whether the damage boost justifies spending a skill point versus upgrading other abilities or 
	 * stats.
	*/
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);

	/*
	 * Calculate the mana cost required to cast the Fire Bolt ability at the next level by calling GetManaCost() with 
	 * the preview level parameter (typically CurrentLevel + 1). This follows the same process as in GetDescription(), 
	 * querying the cost gameplay effect configured in the Blueprint to extract the mana modifier's magnitude value at 
	 * the specified level from its underlying UCurveTable. FMath::Abs() converts the internally negative cost value 
	 * (gameplay effects represent resource costs as negative modifiers) to a positive number for clean UI display, 
	 * preventing confusing tooltip text like "Next Level ManaCost: -30" and instead showing player-friendly values 
	 * like "Next Level ManaCost: 30" that clearly communicate the resource expenditure required at the upgraded level.
	*/
	const float ManaCost = FMath::Abs(GetManaCost(Level));

	/*
	 * Calculate the cooldown duration in seconds for the Fire Bolt ability at the next level by calling GetCooldown() 
	 * with the preview level parameter (typically CurrentLevel + 1). This retrieves the duration magnitude from the 
	 * ability's cooldown gameplay effect configured in the Blueprint, evaluating the UCurveTable or FScalableFloat at 
	 * the specified next level to show players how the cooldown will change if they invest a skill point in upgrading 
	 * the ability. Displaying the next level cooldown helps players make informed upgrade decisions by understanding 
	 * the trade-offs: they can see if leveling up will increase damage and projectile count while also 
	 * potentially increasing cooldown duration, or if the designers have balanced it to reduce cooldown as a reward 
	 * for investing in the ability's progression.
	*/
	const float Cooldown = GetCooldown(Level);
	
	/*
	 * Construct and return a formatted preview string that displays the statistics and behavior of the Fire Bolt ability 
	 * at the next level, shown in a separate UI section to help players evaluate upgrade decisions before spending skill 
	 * points. This method is typically called with (CurrentLevel + 1) as the Level parameter, so when a player is at 
	 * level 3 and selects the ability, this generates a description showing level 4 stats. The 
	 * string uses the same FString::Printf() formatting and rich text markup system as GetDescription(), but with 
	 * "NEXT LEVEL:" as the <Title> header instead of "FIRE BOLT" to clearly distinguish this as preview information 
	 * rather than current stats.
	 * 
	 * The format includes all the same statistical information (Level, ManaCost, Cooldown, 
	 * ScaledDamage) with the same markup tags (<Small>, <Level>, <ManaCost>, <Cooldown>, <Damage>, <Default>) to maintain 
	 * consistent styling across the UI. The projectile count is calculated using FMath::Min(Level, NumProjectiles) to 
	 * show how many bolts will be launched at the next level, respecting the maximum projectile cap defined in 
	 * NumProjectiles.
	*/
	return FString::Printf(TEXT(
		// Title
		"<Title>NEXT LEVEL: </>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Number of FireBolts
		"<Default>Launches %d bolts of fire, "
		"exploding on impact and dealing: </>"

		// Damage
		"<Damage>%d</><Default> fire damage with"
		" a chance to burn</>"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		FMath::Min(Level, NumProjectiles),
		ScaledDamage);
}

void UFoxFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
	bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
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
	
	/*
	 * Convert the rotation (which represents the direction from the socket location to the target location) into a 
	 * normalized direction vector that points forward along that rotation's orientation. FRotator::Vector() creates a 
	 * unit vector (length of 1.0) pointing in the direction the rotation is facing, which will serve as the center line 
	 * of our projectile spread pattern. This forward vector represents the "straight ahead" direction where a single 
	 * projectile would travel, and it becomes the reference point from which we calculate the left and right boundaries 
	 * of the spread cone for multi-projectile patterns.
	*/
	const FVector Forward = Rotation.Vector();
	
	/*
	 * Calculate the actual number of projectiles to spawn for this ability activation by taking the minimum value 
	 * between NumProjectiles (the designer-configured maximum projectile count from the parent UFoxProjectileSpell 
	 * class) and the current ability level returned by GetAbilityLevel(). This creates a level-gating progression 
	 * system where players start with fewer projectiles at low levels and gradually unlock more as they invest skill 
	 * points to increase the ability's level. For example, if NumProjectiles is set to 5, a level 1 Fire Bolt will 
	 * spawn only 1 projectile (min of 5 and 1), level 2 spawns 2 projectiles (min of 5 and 2), level 3 spawns 3, and 
	 * so on until level 5+ where it caps at 5 projectiles.
	*/
	const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());

	/*
	 * Generate an array of evenly-distributed rotators that define the directional spread pattern for spawning multiple 
	 * projectiles. UFoxAbilitySystemLibrary::EvenlySpacedRotators() is a custom static utility function that calculates 
	 * rotations arranged in a cone-shaped spread pattern, taking four parameters:
	 * 1. Forward - The center direction vector (calculated above) that represents the "straight ahead" aim direction
	 * 2. FVector::UpVector - The world up vector (0, 0, 1) used as the axis around which the spread rotations are 
	 *    calculated, ensuring projectiles fan out horizontally in an arc
	 * 3. ProjectileSpread - The total angle in degrees (configured in the Blueprint, default 90°) that defines the 
	 *    width of the cone. Projectiles will be distributed across this angular range
	 * 4. EffectiveNumProjectiles - The number of rotations to generate (calculated above based on ability level)
	 *
	 * For example, with ProjectileSpread = 90° and EffectiveNumProjectiles = 5, this generates 5 rotators spread across 
	 * a 90-degree arc: one pointing straight ahead (center), two angled to the left at -22.5° and -45° from center, and 
	 * two angled to the right at +22.5° and +45°. If only 1 projectile is spawned (level 1), it returns a single rotator 
	 * pointing straight forward with no spread. This mathematical distribution ensures projectiles form a visually 
	 * pleasing fan pattern.
	*/
	TArray<FRotator> Rotations = UFoxAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);

	/*
	 * Iterate through each calculated rotation in the Rotations array to spawn individual projectiles with their 
	 * respective directional offsets. This range-based for loop processes each FRotator element (aliased as 'Rot') 
	 * that was generated by EvenlySpacedRotators(), with each rotation representing a unique direction within the spread 
	 * pattern. Using 'const FRotator&' passes each element by constant reference for performance efficiency (avoiding 
	 * unnecessary copies of the FRotator struct) while preventing accidental modification.
	*/
	for (const FRotator& Rot : Rotations)
	{
		// Variable for the world location where the projectile will be spawned
		FTransform SpawnTransform;
	
		// Set the location of the spawn transform to the socket location (the tip of the character's weapon)
		SpawnTransform.SetLocation(SocketLocation);
	
		/*
		 * Set the projectile rotation
		 * Sets the rotation of the spawn transform using a Quaternion representation of our Rot variable.
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
		SpawnTransform.SetRotation(Rot.Quaternion());
		
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
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		/*
		 * Configure the projectile's damage parameters before it begins play.
		 *
		 * MakeDamageEffectParamsFromClassDefaults() is a helper function (defined in the parent of the parent class
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
		 * projectile begins play and starts detecting collisions.
		*/
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		/*
		* Checks if the value of the bLaunchHomingProjectiles member variable is true
		* This boolean variable (value set in the Blueprint), allowing designers to easily switch between "dumb fire" 
		* projectiles (straight shots) and "smart" tracking projectiles (homing shots) without changing any other code 
		* or projectile configuration, enabling abilities to be upgraded from basic projectiles to guided projectiles 
		* through progression systems or skill trees.
		*/
		if (bLaunchHomingProjectiles)
		{
			/*
			 * Configure the projectile's homing behavior by setting up the target tracking system used by the 
			 * ProjectileMovementComponent. The HomingTarget parameter (passed into this function) determines whether 
			 * the projectile will track a specific actor or fly toward a static world location. This branching logic 
			 * handles two distinct homing scenarios:
			 *
			 * SCENARIO 1: Actor-Targeted Homing (HomingTarget is valid and implements CombatInterface)
			 * When HomingTarget exists and properly implements the ICombatInterface, the projectile will continuously 
			 * track the target actor's position throughout its flight.
			 *
			 * SCENARIO 2: Location-Targeted Homing (HomingTarget is null or doesn't implement CombatInterface)
			 * When no valid HomingTarget exists, the projectile will instead track toward a fixed world location specified 
			 * by ProjectileTargetLocation (typically the cursor hit location from player input).
			*/
			if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
			{
				/*
				 * Assign the target actor's root component to the projectile's homing system, enabling the 
				 * ProjectileMovementComponent to track the actor's world position for trajectory calculations. 
				 * GetRootComponent() returns the top-level USceneComponent in the actor's component hierarchy 
				 * (typically the capsule component for characters or the root scene component for simpler actors), 
				 * which serves as the authoritative position reference that automatically updates as the target 
				 * moves through the world. The ProjectileMovementComponent uses this component's transform each 
				 * tick to calculate the direction vector from the projectile's current position to the target's 
				 * position, then applies acceleration in that direction (scaled by HomingAccelerationMagnitude) to 
				 * curve the projectile's flight path. This creates dynamic tracking behavior where projectiles 
				 * continuously adjust their trajectory to follow moving enemies, dodging players, or fleeing NPCs, 
				 * producing the classic "heat-seeking missile" effect where the projectile arcs through the air to 
				 * chase its target.
				*/
				Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
			}
			// If the above condition is not met
			else
			{
				/*
				 * Create a "dummy" scene component to serve as a stationary position marker for location-based homing.
				 * NewObject<USceneComponent>() instantiates a new USceneComponent owned by the projectile. The 
				 * USceneComponent::StaticClass() parameter returns a UClass* pointer that tells NewObject what type of 
				 * object to construct. This creates a lightweight spatial anchor that holds a world position without any 
				 * visual representation, ticking, or world interaction. It's simply a transform holder that the 
				 * ProjectileMovementComponent can query for position data during homing calculations.
				*/
				Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());

				/*
				 * Position the dummy scene component at the target world location specified by ProjectileTargetLocation 
				 * (typically the cursor hit point from player input). SetWorldLocation() places this component at the 
				 * exact 3D coordinates in world space where the projectile should arc toward, establishing a fixed spatial 
				 * anchor that won't move during the projectile's flight. Unlike actor-targeted homing (where the target's 
				 * root component updates as the enemy moves), this dummy component remains stationary at its initial 
				 * position, causing the projectile to smoothly curve toward that specific ground location, wall point, or 
				 * cursor position where the player aimed.
				*/
				Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);

				/*
				 * Assign the dummy scene component to the ProjectileMovementComponent's homing system, enabling location-based 
				 * tracking identical to actor-based tracking. By setting HomingTargetComponent to our custom scene component, 
				 * the ProjectileMovementComponent's homing logic will query this component's world location each tick to 
				 * calculate the direction vector from the projectile's current position to the target position, then apply 
				 * acceleration toward that location (scaled by HomingAccelerationMagnitude).
				*/
				Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
			}
			/*
			 * HomingAccelerationMagnitude determines the rate at which the projectile can change direction to home in on its 
			 * target, measured in unreal units per second squared (cm/s²). Higher values create tighter turning circles and 
			 * more aggressive tracking (projectiles snap quickly toward moving targets), while lower values produce gentle 
			 * arcing trajectories that feel more like guided missiles than instant-turn magic bolts. We randomize this value 
			 * using FMath::FRandRange() between HomingAccelerationMin and HomingAccelerationMax (configured in the Blueprint) 
			 * to introduce visual variety. Each projectile in a multi-shot spread will curve at slightly different rates, 
			 * preventing the "robotic uniformity" of identical homing paths.
			*/
			Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
			
			/*
			* bIsHomingProjectile is the master toggle that activates or deactivates the entire homing system on the 
			* ProjectileMovementComponent. When true, the component calculates acceleration vectors toward HomingTargetComponent 
			* every tick and applies them to the projectile's velocity, causing it to arc smoothly toward the target. When 
			* false, the projectile flies in a straight line along its initial launch direction, ignoring the HomingTargetComponent 
			* entirely even if it's set.
			*/
			Projectile->ProjectileMovement->bIsHomingProjectile = true;
		}
		// Finish spawning the projectile at the specified transform
		Projectile->FinishSpawning(SpawnTransform);
	}
}
