// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

class UMVVM_LoadSlot;

/**
 * 
 */
UCLASS()
class FOX_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	// Initializes the three load slot view models and registers them in the LoadSlots map
	void InitializeLoadSlots();

	// The class type used to create load slot view model instances. We set this value in the blueprint derived from this class 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	// Retrieves the load slot view model at the specified index from the LoadSlots map
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;
	
	// Handles creating a new save slot by saving the entered player name and initializing the slot to update its UI
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);

	// Handles the new game button press by switching the UI to show the name entry widget
	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	// Handles selecting an existing save slot to load the game from
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);
	
	// Loads saved data from disk for all save slots and updates their view models with player names and slot statuses
	void LoadData();

private:

	// Map storing all load slot view models indexed by slot number for quick access and iteration
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	// View model for the first save slot (index 0)
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;

	// View model for the second save slot (index 1)
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;

	// View model for the third save slot (index 2)
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;
};
