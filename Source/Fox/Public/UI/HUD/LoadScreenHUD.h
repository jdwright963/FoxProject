// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoadScreenHUD.generated.h"

class UMVVM_LoadScreen;
class ULoadScreenWidget;
/**
 * 
 */
UCLASS()
class FOX_API ALoadScreenHUD : public AHUD
{
	GENERATED_BODY()
public:
	
	// The Blueprint class type for the load screen widget (set this in the Blueprint derived from ALoadScreenHUD)
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadScreenWidgetClass;

	// The instantiated load screen widget that displays the save slot UI to the player
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadScreenWidget> LoadScreenWidget;

	// The Blueprint class type for the load screen view model (set this in the Blueprint derived from ALoadScreenHUD)
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadScreen> LoadScreenViewModelClass;

	// The instantiated load screen view model that manages the data and logic for save slots
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadScreen> LoadScreenViewModel;

protected:
	// Creates and initializes the load screen view model and widget, then displays them to the player
	virtual void BeginPlay() override;
};
