// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
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
			"<Title>Arcane Shards</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Summon a shard of arcane energy, "
			"causing radial arcane damage of  </>"

			// Damage
			"<Damage>%d</><Default> at the shard origin.</>"),

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
			"<Title>Arcane Shards</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Summon %d shards of arcane energy, causing radial arcane damage of </>"

			// Damage
			"<Damage>%d</><Default> at the shard origins.</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShards),
			ScaledDamage);
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 Level)
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
		"<Title>Next Level:</>\n\n"

		// Level
		"<Small>Level: </><Level>%d</>\n"
		// ManaCost
		"<Small>ManaCost: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Addition Number of Shock Targets
		"<Default>Summon %d shards of arcane energy, causing radial arcane damage of </>"

		// Damage
		"<Damage>%d</><Default> at the shard origins.</>"),

		// Values
		Level,
		ManaCost,
		Cooldown,
		FMath::Min(Level, MaxNumShards),
		ScaledDamage);
}
