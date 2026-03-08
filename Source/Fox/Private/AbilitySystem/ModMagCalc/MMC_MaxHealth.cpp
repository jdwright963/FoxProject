// Copyright TryingToMakeGames


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/FoxAttributeSet.h"
#include "Curves/BezierUtilities.h"
#include "Interaction/CombatInterface.h"

// Needs comments
UMMC_MaxHealth::UMMC_MaxHealth()
{
	// GetVigorAttribute is a static function in FoxAttributeSet that is created by the ATTRIBUTE_ACCESSORS macro
	VigorDef.AttributeToCapture = UFoxAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;
	
	RelevantAttributesToCapture.Add(VigorDef);
}

// Needs comments
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Gather tags from source and target
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);
	
	// Initialize PlayerLevel to 1 as a safe default value in case we can't retrieve the actual level from the source object
	int32 PlayerLevel = 1;
	
	// Check if the source object (retrieved from the gameplay effect context) implements UCombatInterface before 
	// attempting to call interface functions to prevent crashes.
	//
	// UNREAL INTERFACE PATTERN: UCombatInterface vs ICombatInterface
	// 
	// Unreal generates TWO classes for each interface:
	// 1. UCombatInterface (U-prefixed): UObject-based class used for reflection, blueprint integration, and runtime type checking
	// 2. ICombatInterface (I-prefixed): Pure C++ interface class that contains the actual interface function declarations
	//
	// We use Implements<UCombatInterface>() for the runtime type check because:
	// - Implements<>() requires the UObject-based version (U-prefix) to work with Unreal's reflection system
	// - This allows the engine to check if the object implements the interface at runtime safely
	//
	// We use ICombatInterface::Execute_GetPlayerLevel() for the function call because:
	// - Execute_ functions are auto-generated static functions for BlueprintNativeEvent methods
	// - These Execute_ functions are generated on the native C++ interface class (I-prefix)
	// - Execute_ properly handles both C++ and Blueprint implementations of the interface function
	// - It will call the Blueprint version if overridden, or fall back to the _Implementation C++ version
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		// Retrieve the actual player level from the source object by calling GetPlayerLevel() through the 
		// ICombatInterface using the proper Execute_ syntax (this is an auto generated static function) for
		// BlueprintNativeEvent functions
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}
