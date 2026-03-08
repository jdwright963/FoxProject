// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "FoxAIController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class FOX_API AFoxAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	// Constructor for the AFoxAIController class
	AFoxAIController();
	
protected:
	
	// Variable for the Behavior Tree Component that will be used by this AI Controller.
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
