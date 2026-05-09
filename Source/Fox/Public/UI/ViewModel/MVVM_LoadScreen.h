// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

// Dynamic multicast delegate that broadcasts when a save slot is selected, allowing multiple listeners to respond to slot selection events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

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
	
	// Blueprint-assignable delegate that broadcasts when a save slot is selected, allowing Blueprint to bind and respond to slot selection events
	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;

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

	// Handles selecting an existing save slot to load the game from that slot
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);
	
	// Handles deleting the currently selected save slot by removing its saved data from disk
	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();
	
	// Handles the play button press by loading the game from the currently selected save slot and traveling to its associated map
	UFUNCTION(BlueprintCallable)
	void PlayButtonPressed();
	
	// Loads saved data from disk for all save slots and updates their view models with player names and slot statuses
	void LoadData();
	
	// Sets the number of load slots displayed in the UI and triggers MVVM field change notification to update bound widgets
	void SetNumLoadSlots(int32 InNumLoadSlots);

	// Retrieves the current number of load slots configured for the load screen UI
	int32 GetNumLoadSlots() const { return NumLoadSlots; }

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
	
	// The currently selected load slot view model, tracking which save slot the player has chosen for loading, deleting, or overwriting
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> SelectedSlot;
	
	// The configurable number of save slots displayed in the load screen UI, with MVVM field change notifications enabled for reactive UI updates
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	int32 NumLoadSlots;
};
