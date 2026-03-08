// Copyright TryingToMakeGames


#include "AI/FoxAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AFoxAIController::AFoxAIController()
{
	//Create a Blackboard Component that will be instanced inside all instances of this class. 
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	
	// Crash the game if the blackboard component is invalid
	check(Blackboard);
	
	//Create a Behavior Tree Component that will be instanced inside all instances of this class.
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	
	// Crash the game if the Behavior Tree Component is invalid
	check(BehaviorTreeComponent);
}
