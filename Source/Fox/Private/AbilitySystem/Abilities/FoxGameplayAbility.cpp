// Copyright TryingToMakeGames


#include "AbilitySystem/Abilities/FoxGameplayAbility.h"

#include "AbilitySystem/FoxAttributeSet.h"

FString UFoxGameplayAbility::GetDescription(int32 Level)
{
	// Returns a formatted string with rich text tags: <Default> for default styled text and <Level> for level-specific
	// styling. This is the default implementation showing placeholder Lorem Ipsum text and the ability level.
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum "
															  "LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum", Level);
}

FString UFoxGameplayAbility::GetNextLevelDescription(int32 Level)
{
	// Returns a formatted string describing the next level benefits using rich text tags. <Default> tags wrap normal
	// descriptive text, <Level> tags wrap the level number, and \n creates a line break in the UI display.
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FString UFoxGameplayAbility::GetLockedDescription(int32 Level)
{
	// Returns a formatted string indicating the ability is locked, using <Default> rich text tags for styling
	// and showing the required level needed to unlock the ability.
	return FString::Printf(TEXT("<Default>Spell Locked Until Level: %d</>"), Level);
}

float UFoxGameplayAbility::GetManaCost(float InLevel) const
{
	// Initialize a local float variable to 0.f that will store the calculated mana cost value. This serves as both 
	// the default return value if no cost effect is found or if the mana modifier doesn't exist, and as the output 
	// parameter that will be populated by GetStaticMagnitudeIfPossible when the mana attribute modifier is found
	float ManaCost = 0.f;
	
	// Retrieve the UGameplayEffect that defines this ability's cost and store it in a const pointer. GetCostGameplayEffect() 
	// is inherited from UGameplayAbility and returns the effect specified in the ability's CostGameplayEffectClass property. 
	// This if statement performs both retrieval and validation: if GetCostGameplayEffect() returns a valid pointer (non-null), 
	// CostEffect is assigned that value and the condition evaluates to true. This cost gameplay effect is set in the 
	// BP for the ability
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		// Iterate through each FGameplayModifierInfo element in the cost effect's Modifiers array to search for the 
		// specific modifier that affects the Mana attribute. Each modifier in this array defines how the effect modifies 
		// a particular attribute, and we need to find the one that corresponds to mana consumption so we can extract its 
		// magnitude value which represents the actual mana cost at the given ability level
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			// Check if the current modifier's Attribute property (which identifies what attribute this modifier affects) 
			// matches the Mana attribute from UFoxAttributeSet by comparing it with GetManaAttribute(). If this condition 
			// is true, we've found the modifier that defines mana cost
			if (Mod.Attribute == UFoxAttributeSet::GetManaAttribute())
			{
				// Call GetStaticMagnitudeIfPossible on the modifier's magnitude calculation object, passing the ability 
				// level (InLevel) as the first parameter and the ManaCost variable as the second out parameter. This 
				// evaluates the magnitude calculation (which may be a curve, scalable float, or other calculation) at 
				// the specified level and populates the ManaCost variable with the resulting mana cost value
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);

				// Exit the loop immediately after finding and processing the Mana attribute modifier since we only need 
				// one mana cost value and there should only be one mana modifier in the cost effect
				break;
			}
		}
	}
	// Return the calculated mana cost value extracted from the cost effect's mana modifier, or 0.f if no cost effect 
	// was found or if the mana attribute modifier didn't exist in the effect's modifiers array
	return ManaCost;
}

float UFoxGameplayAbility::GetCooldown(float InLevel) const
{
	// Initialize a local float variable to 0.f that will store the calculated cooldown duration value. This serves as 
	// both the default return value if no cooldown effect is found, and as the output parameter that will be populated 
	// by GetStaticMagnitudeIfPossible when the cooldown duration is successfully retrieved from the effect
	float Cooldown = 0.f;

	// Retrieve the UGameplayEffect that defines this ability's cooldown and store it in a const pointer. 
	// GetCooldownGameplayEffect() is inherited from UGameplayAbility and returns the effect specified in the ability's 
	// CooldownGameplayEffectClass property. This if statement performs both retrieval and validation: if 
	// GetCooldownGameplayEffect() returns a valid pointer (non-null), CooldownEffect is assigned that value and the 
	// condition evaluates to true. This cooldown gameplay effect is set in the BP for the ability
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		// Call GetStaticMagnitudeIfPossible on the cooldown effect's DurationMagnitude property, passing the ability 
		// level (InLevel) as the first parameter and the Cooldown variable as the second out parameter. This evaluates 
		// the duration magnitude calculation (which may be a curve, scalable float, or other calculation) at the 
		// specified level and populates the Cooldown variable with the resulting cooldown duration value in seconds
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	// Return the calculated cooldown duration value extracted from the cooldown effect's duration magnitude, or 0.f if 
	// no cooldown effect was found or if the duration magnitude couldn't be evaluated
	return Cooldown;
}
