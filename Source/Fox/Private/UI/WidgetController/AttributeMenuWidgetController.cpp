// Copyright TryingToMakeGames


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/FoxPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	// Verify that AttributeInfo data asset is not null. This crashes in development builds if it's missing,
	// helping catch configuration errors early during development
	check(AttributeInfo);

	// Iterate through all gameplay attributes defined in the TagsToAttributes map (e.g., Strength, Intelligence, etc.)
	// to set up listeners for each attribute's value changes
	for (auto& Pair: GetFoxAS()->TagsToAttributes)
	{
		// Subscribe to value change notifications for this specific attribute - whenever the attribute's value changes
		// (through gameplay effects, abilities, etc.), the lambda function will be called to update the UI
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
		[this, Pair] (const FOnAttributeChangeData& Data)
		{
			//Pair.Value is a function pointer and putting () calls that function
			BroadcastAttributeInfo(Pair.Key, Pair.Value());
		}
		);
	}
	
	// Use the getter function that this class inherits to get the FoxPlayerState.
	// Subscribe a lambda (a function defined right here) to the player state's OnAttributePointsChangedDelegate, so whenever attribute points
	// are modified (through replication or direct calls), this widget controller is notified and can update the UI
	GetFoxPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			// Broadcast the attribute points value using this widget controller's own delegate, allowing UI widgets
			// subscribed to AttributePointsChangedDelegate to update their display with the new points value
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	UFoxAttributeSet* AS = CastChecked<UFoxAttributeSet>(AttributeSet); 
	check(AttributeInfo);

	// Iterate through the TagsToAttributes map to broadcast all attribute information
	// We use a reference (auto&) instead of a pointer for Pair because:
	// 1. References are safer - they cannot be null and don't require null checks
	// 2. Cleaner syntax - no need to use -> or dereference operators
	// 3. More efficient - avoids unnecessary pointer indirection
	// 4. The map guarantees the Pair exists during iteration, making references safe to use
	for (auto& Pair: AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
	
	// Use the getter function that this class inherits to get the FoxPlayerState.
	// Broadcast the attribute points value retrieved from the player state to all subscribed UI widgets.
	// This ensures the attribute menu displays the correct number of available attribute points when opened,
	// similar to how we broadcast initial attribute values above. The delegate notifies all listeners of the current points.
	AttributePointsChangedDelegate.Broadcast(GetFoxPS()->GetAttributePoints());
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	// Cast the generic AbilitySystemComponent pointer to our project-specific UFoxAbilitySystemComponent type using CastChecked,
	// which performs a safe cast and crashes if AbilitySystemComponent is null or not of type UFoxAbilitySystemComponent.
	// This allows us to access Fox-specific functionality like UpgradeAttribute() that isn't available on the base UAbilitySystemComponent class.
	UFoxAbilitySystemComponent* FoxASC = CastChecked<UFoxAbilitySystemComponent>(AbilitySystemComponent);

	// Call the UpgradeAttribute function on the Fox-specific ability system component, passing the attribute tag
	// that identifies which attribute to upgrade (e.g., Strength, Intelligence, etc.).
	// The ASC handles the actual upgrade logic: checking available points, spending them, and increasing the attribute value.
	FoxASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	// Retrieve the static attribute information (name, description, tag) from the data asset
	// using the gameplay tag as the lookup key
	FFoxAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
		
	// Get the current runtime value of this attribute from the attribute set
	// then we get its numeric value.
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
		
	// Broadcast the complete attribute information (static data + current value) to all listening widgets
	// so they can display the attribute details to the player. See AttributeInfo.h for definition of the data asset
	AttributeInfoDelegate.Broadcast(Info);
}
