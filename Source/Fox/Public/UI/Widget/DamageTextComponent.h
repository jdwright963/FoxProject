// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	
	// Function to set the damage text displayed by the component. It is called
	// when the component is created and it is implemented in the blueprint for 
	// this component. The blueprint that inherits from this class is created in the editor and in the 
	// details panel the widget property for this widget component is set to WBP_DamageText
	// is spawned from C++ and this function is called to set the text.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float DamageText, bool bBlockedHit, bool bCriticalHit);
};
