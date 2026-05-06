// Copyright TryingToMakeGames


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	// TODO: Check slot status based on loaded data
	SetWidgetSwitcherIndex.Broadcast(2);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	// Updates the PlayerName property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	// Updates the LoadSlotName property with the new value and automatically notifies any bound UI elements through Unreal's MVVM system
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
