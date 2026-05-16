// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

// Dynamic multicast delegate that broadcasts widget switcher index changes with a single int32 parameter
// Used to switch between different UI states in the load slot widget (e.g., between slot display view and name entry view)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);

// Dynamic multicast delegate that broadcasts when the select slot button should be enabled or disabled based on slot state
// Used to control the interactivity of the select button (e.g., disabled for vacant slots, enabled for slots with saved data)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 
 */
UCLASS()
class FOX_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:
	
	// Delegate that broadcasts widget switcher index changes to update the UI state between different views (e.g., slot display and name entry)
	// BlueprintAssignable allows Blueprint scripts to bind callback functions to this delegate.
	// C++ code can broadcast events, and all Blueprint functions bound to this delegate will be notified.
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;
	
	// Delegate that broadcasts when the select slot button should be enabled or disabled based on the slot's current state
	// BlueprintAssignable allows Blueprint scripts to bind callback functions to this delegate.
	// C++ code can broadcast enable/disable events, and all Blueprint functions bound to this delegate will be notified
	// to control the interactivity of the select button (e.g., disabled for vacant slots, enabled for slots with saved data)
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	// Refreshes the slot's UI state and data after it has been saved or modified
	void InitializeSlot();

	// Integer representing the index of this slot (e.g., "0", "1", "2")
	UPROPERTY()
	int32 SlotIndex;
	
	// The current status/state of this save slot (e.g., vacant, entered name, taken) used to determine UI display and interaction behavior
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;
	
	// The tag identifier used to determine which PlayerStart actor to spawn the player at when loading this save slot
	UPROPERTY()
	FName PlayerStartTag;
	
	// The asset path/name of the map/level associated with this save slot, used to load the correct map when the player loads this save
	UPROPERTY()
	FString MapAssetName;

	/** Field Notifies */

	// Sets the player's name for this save slot and notifies bound UI elements of the change
	void SetPlayerName(FString InPlayerName);
	
	// Sets the map name for this save slot and notifies bound UI elements of the change
	void SetMapName(FString InMapName);
	
	// Sets the player's level for this save slot and notifies bound UI elements of the change
	void SetPlayerLevel(int32 InLevel);

	// Sets the unique save slot name identifier and notifies bound UI elements of the change
	void SetLoadSlotName(FString InLoadSlotName);
	
	// Returns the player name associated with this save slot, used for displaying in the UI
	FString GetPlayerName() const { return PlayerName; }
	
	// Returns the map name associated with this save slot, used for displaying in the UI
	FString GetMapName() const { return MapName; }

	// Returns the player's current level associated with this save slot, used for displaying character progression in the UI
	int32 GetPlayerLevel() const { return PlayerLevel; }

	// Returns the unique save slot identifier name used for loading and saving game data
	FString GetLoadSlotName() const { return LoadSlotName; }

private:
	
	// The name of the player associated with this save slot, displayed in the UI and used for identification
	// FieldNotify: Enables MVVM binding - automatically notifies bound UI elements when the value changes
	// Setter: Specifies that SetPlayerName() function (must be named exactly like this) should be used to modify this 
	// property (enables proper notification)
	// Getter: Specifies that GetPlayerName() function (must be named exactly like this) should be used to read this property
	// meta = (AllowPrivateAccess = "true"): Allows Blueprint access to this private member variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FString PlayerName;
	
	// The name of the map/level associated with this save slot, displayed in the UI to show where the player's save is located
	// FieldNotify: Enables MVVM binding - automatically notifies bound UI elements when the value changes
	// Setter: Specifies that SetMapName() function (must be named exactly like this) should be used to modify this
	// property (enables proper notification)
	// Getter: Specifies that GetMapName() function (must be named exactly like this) should be used to read this property
	// meta = (AllowPrivateAccess = "true"): Allows Blueprint access to this private member variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString MapName;
	
	// The player's current level associated with this save slot, displayed in the UI to show character progression
	// FieldNotify: Enables MVVM binding - automatically notifies bound UI elements when the value changes
	// Setter: Specifies that SetPlayerLevel() function (must be named exactly like this) should be used to modify this
	// property (enables proper notification)
	// Getter: Specifies that GetPlayerLevel() function (must be named exactly like this) should be used to read this property
	// meta = (AllowPrivateAccess = "true"): Allows Blueprint access to this private member variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	int32 PlayerLevel;

	// The unique identifier name for this save slot (e.g., "LoadSlot_0"), used for loading and saving game data
	// FieldNotify: Enables MVVM binding - automatically notifies bound UI elements when the value changes
	// Setter: Specifies that SetLoadSlotName() function (must be named exactly like this) should be used to modify this
	// property (enables proper notification)
	// Getter: Specifies that GetLoadSlotName() function (must be named exactly like this) should be used to read this property
	// meta = (AllowPrivateAccess = "true"): Allows Blueprint access to this private member variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FString LoadSlotName;
};
