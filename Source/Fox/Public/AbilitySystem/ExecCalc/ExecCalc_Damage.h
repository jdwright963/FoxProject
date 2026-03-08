// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	
	// Constructor of this class
	UExecCalc_Damage();
	
	/**
	 * Determines and applies debuff effects to the target based on the damage type and debuff chance.
	 * This function evaluates debuff probability using the source's debuff chance attribute and applies
	 * the appropriate debuff effect (stun, burn, arcane, physical) if the random roll succeeds.
	 * 
	 * @param ExecutionParams - The execution parameters containing source and target ability system components,
	 *                          used to access attribute sets and apply gameplay effects.
	 * @param Spec - The gameplay effect specification containing the damage type tag that determines which
	 *               debuff type to potentially apply (e.g., Damage.Fire -> Debuff.Burn).
	 * @param EvaluationParameters - Parameters used for evaluating captured attributes, including any modifiers
	 *                               or tags that affect the evaluation context.
	 * @param InTagsToDefs - A map of gameplay tags to their corresponding attribute capture definitions, used
	 *                       to retrieve the debuff chance attribute from the source character.
	 */
	void DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
						 const FGameplayEffectSpec& Spec,
						 FAggregatorEvaluateParameters EvaluationParameters,
						 const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const;
	
	// This is the function that defines what happens when this execution calculation is executed
	// Calculates damage values based on source and target attributes, then applies the result to the target
	// When making a gameplay effect and choosing Custom Calculation Class for its Magnitude Calculation Type parameter
	// This is the class that will determine how attributes will be effected by the gameplay effect
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
