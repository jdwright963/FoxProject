// Copyright TryingToMakeGames


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	// Retrieves the underlying integer value from the SlotStatus enum to use as the widget switcher index
	const int32 WidgetSwitcherIndex = SlotStatus.GetValue();
	
	// Broadcasts the index to any listening UI elements to switch which widget is displayed (e.g., empty slot, name entry, or existing save)
	SetWidgetSwitcherIndex.Broadcast(WidgetSwitcherIndex);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	// Updates the PlayerName property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

void UMVVM_LoadSlot::SetMapName(FString InMapName)
{
	// Updates the MapName property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(MapName, InMapName);
}

void UMVVM_LoadSlot::SetPlayerLevel(int32 InLevel)
{
	// Updates the PlayerLevel property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(PlayerLevel, InLevel);
}

void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	// Updates the LoadSlotName property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
