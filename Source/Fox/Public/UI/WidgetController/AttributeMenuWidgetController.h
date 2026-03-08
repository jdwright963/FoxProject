// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/FoxWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

struct FGameplayAttribute;
struct FGameplayTag;
class UAttributeInfo;
struct FFoxAttributeInfo;

// Needs comment and explain why the input parameter has to be a reference or else it cannot be forward declared
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FFoxAttributeInfo&, Info);

/**
 * Needs comment
 */
UCLASS(BlueprintType, Blueprintable)
class FOX_API UAttributeMenuWidgetController : public UFoxWidgetController
{
	GENERATED_BODY()
public:
	
	// Sets up callbacks that listen for attribute value changes and attribute points changes
	// This function subscribes to the attribute set's value change delegates for each attribute,
	// and also subscribes to the player state's attribute points changed delegate
	// When any of these values change, it broadcasts the updated information to the UI
	virtual void BindCallbacksToDependencies() override;

	// Broadcasts the initial values of all attributes and attribute points when the widget is first created (In this
	// project every time the attribute menu is opened).
	// This ensures the UI displays the correct current values as soon as it becomes visible
	// Iterates through all attributes in the attribute set and broadcasts their information,
	// then broadcasts the current available attribute points from the player state
	virtual void BroadcastInitialValues() override;
	
	// Instance of the delegate defined at the top of the file
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;
	
	// Delegate that broadcasts when the player's available attribute points change
	// This is triggered when points are earned (leveling up) or spent (upgrading attributes)
	// Listening widgets can update their UI to reflect the current number of available points
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnPlayerStatChangedSignature AttributePointsChangedDelegate;
	
	// Called from the UI (Blueprint) when the player clicks to upgrade a specific attribute
	// Forwards the upgrade request to the ability system component, which applies the upgrade
	// The AttributeTag parameter identifies which attribute to upgrade (e.g., Strength, Intelligence, etc.)
	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	
protected:

	// Instance of the class defined in AttributeInfo.h this will be set in the blueprint
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;
	
private:
	
	// Helper function that retrieves attribute information from the data asset and broadcasts it to the UI
	// Takes an attribute tag to look up static info (name, description) and a gameplay attribute to get the current value
	// Combines both static and runtime data into a complete attribute info struct and broadcasts it via AttributeInfoDelegate
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
};
