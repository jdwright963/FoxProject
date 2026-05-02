// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxFireBlast.h"

#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Actor/FoxFireBall.h"

FString UFoxFireBlast::GetDescription(int32 Level)
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
		"<Title>Fire Blast</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
		// Number of Fire Balls
		"<Default>Launches %d </>"
		"<Default>fire balls in all directions, each coming back and </>"
		"<Default>exploding upon return, causing </>"

		// Damage
		"<Damage>%d</><Default> radial fire damage with"
		" a chance to burn</>"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		NumFireBalls,
		ScaledDamage);
}

FString UFoxFireBlast::GetNextLevelDescription(int32 Level)
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
			"<Title>NEXT LEVEL:</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Number of Fire Balls
			"<Default>Launches %d </>"
			"<Default>fire balls in all directions, each coming back and </>"
			"<Default>exploding upon return, causing </>"

			// Damage
			"<Damage>%d</><Default> radial fire damage with"
			" a chance to burn</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			NumFireBalls,
			ScaledDamage);
}

TArray<AFoxFireBall*> UFoxFireBlast::SpawnFireBalls()
{
	/*
	 * Create an empty TArray to store pointers to all spawned AFoxFireBall actors, which will be returned at the
	 * end of this function to allow the caller to track and manipulate the created fireballs (e.g., for animation,
	 * movement patterns, or cleanup).
	 */
	TArray<AFoxFireBall*> FireBalls;

	/*
	 * Get the forward direction vector of the avatar actor (the character or pawn that owns this ability) to use
	 * as the reference direction for calculating the spawn rotations of the fireballs in a radial pattern around
	 * the character.
	 */
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();

	/*
	 * Get the world space location of the avatar actor to use as the spawn point for all fireballs, ensuring they
	 * originate from the character's position rather than the world origin or some other arbitrary location.
	 */
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();

	/*
	 * Calculate an array of FRotators evenly distributed in a 360-degree circle around the avatar's forward
	 * direction using the UpVector as the rotation axis, with the number of rotators matching NumFireBalls so each
	 * fireball will be launched in a unique direction forming a radial blast pattern.
	 */
	TArray<FRotator> Rotators = UFoxAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	/*
	 * Iterate through each rotator in the array
	 */
	for (const FRotator& Rotator : Rotators)
	{
		/*
		 * Create an FTransform object that will define both the spawn position and orientation for the fireball
		 * actor in world space, encapsulating location, rotation, and scale into a single struct that can be passed
		 * to the spawning function.
		 */
		FTransform SpawnTransform;

		/*
		 * Set the location component of the spawn transform to the avatar's current world position so the fireball
		 * originates from the character rather than at the world origin or an arbitrary location.
		 */
		SpawnTransform.SetLocation(Location);

		/*
		 * Set the rotation component of the spawn transform by converting the current rotator (which defines the
		 * fireball's launch direction in the radial pattern) to a quaternion representation, which is the internal
		 * format used by FTransform for storing rotations.
		 */
		SpawnTransform.SetRotation(Rotator.Quaternion());
		
		/*
		 * Spawn a fireball actor using deferred spawning (SpawnActorDeferred) which creates the actor instance but
		 * delays calling BeginPlay and initializing components until FinishSpawning is called, allowing us to
		 * configure the fireball's properties (like DamageEffectParams) before it becomes fully active in the world.
		 * The parameters specify: FireBallClass as the actor type to spawn, SpawnTransform for its world position
		 * and rotation, the ability owner as the spawned actor's owner for replication and authority purposes, the
		 * player controller's pawn as the instigator for damage attribution and team identification, and
		 * AlwaysSpawn to ensure the actor spawns even if there are collisions at the spawn location.
		 */
		AFoxFireBall* FireBall = GetWorld()->SpawnActorDeferred<AFoxFireBall>( 
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		/*
		 * Populate the fireball's DamageEffectParams struct by calling MakeDamageEffectParamsFromClassDefaults()
		 * (inherited from UFoxDamageGameplayAbility parent class) which constructs a damage parameter struct
		 * containing the ability's configured (set in the blueprint) damage values, debuff chances, knockback settings,
		 * and other effect properties that will be applied when the fireball hits a target, ensuring each spawned fireball deals
		 * the correct amount of damage and applies the appropriate gameplay effects based on the ability's current
		 * level and configuration.
		 */
		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		/*
		 * Set the fireball's ReturnToActor variable to reference the avatar actor (the character or pawn that owns
		 * this ability) so the fireball knows which actor to return to after traveling outward to its maximum range,
		 * enabling the boomerang-like behavior where each fireball launches away from the character in its assigned
		 * direction, then curves back toward the caster and explodes upon reaching them, creating the signature
		 * Fire Blast pattern of outgoing and returning projectiles.
		 */
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
		
		/*
		 * Set the fireball actor's owner to the avatar actor (the character or pawn that owns this ability) to
		 * establish proper ownership hierarchy for network replication and authority purposes, ensuring the server
		 * knows which player/character spawned this projectile for damage attribution, team identification, and
		 * proper cleanup when the owner is destroyed or disconnected from the game.
		 */
		FireBall->SetOwner(GetAvatarActorFromActorInfo());
		
		/*
		 * Configure the fireball's ExplosionDamageParams with the ability's damage effect settings by calling
		 * MakeDamageEffectParamsFromClassDefaults() to create a separate damage parameter struct specifically for
		 * the explosion that occurs when the fireball returns to the caster, allowing the explosion to use the same
		 * damage values, debuff chances, and effect properties as the initial projectile hit but potentially with
		 * different application logic or area-of-effect behavior defined in the fireball's Blueprint implementation.
		 */
		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();

		/*
		 * Set the fireball actor's owner to the avatar actor (the character or pawn that owns this ability) to
		 * establish proper ownership hierarchy for network replication and authority purposes, ensuring the server
		 * knows which player/character spawned this projectile for damage attribution, team identification, and
		 * proper cleanup when the owner is destroyed or disconnected from the game.
		 */
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		/*
		 * Add the newly spawned fireball actor to the FireBalls array to maintain an array of all fireballs
		 * created during this ability activation, allowing the caller to track, manipulate, or reference them later
		 * (e.g., for synchronized animations, cleanup on ability cancellation, or gameplay logic that depends on
		 * knowing how many projectiles are active).
		 */
		FireBalls.Add(FireBall);

		/*
		 * Complete the deferred spawn process by calling FinishSpawning() with the previously configured
		 * SpawnTransform, which triggers the fireball's BeginPlay(), initializes all components, enables collision
		 * detection, starts any configured movement behavior, and makes the actor fully active in the world so it
		 * can begin traveling toward its target and interacting with other actors.
		 */
		FireBall->FinishSpawning(SpawnTransform);
	}
	/*
	 * Return the array of all spawned AFoxFireBall actors to the caller so they can track, reference, or manipulate
	 * the fireballs after creation (e.g., for applying movement patterns, triggering animations, handling ability
	 * cancellation cleanup, or implementing gameplay logic that needs to know which projectiles are active).
	 */
	return FireBalls;
}
