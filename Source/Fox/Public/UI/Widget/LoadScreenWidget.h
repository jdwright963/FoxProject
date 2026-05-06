// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class FOX_API ULoadScreenWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	/**
	 * Blueprint implementable event called to initialize the widget after it's created and added to viewport.
	 * This allows Blueprint to perform additional setup such as binding to view models or configuring UI elements.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
};
