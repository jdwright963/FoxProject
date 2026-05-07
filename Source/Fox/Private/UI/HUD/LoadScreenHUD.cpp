// Copyright TryingToMakeGames


#include "UI/HUD/LoadScreenHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/Widget/LoadScreenWidget.h"

void ALoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();
	
	// Create a new instance of the load screen view model using the class specified in LoadScreenViewModelClass
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);

	// Initialize the three save slot view models within the load screen view model
	LoadScreenViewModel->InitializeLoadSlots();
	
	/*
	 * Create the load screen widget instance from the specified widget class
	 * CreateWidget is a template function where:
	 * - The template parameter <ULoadScreenWidget> specifies the C++ base class type that the function will return
	 * - The function parameter LoadScreenWidgetClass is a TSubclassOf<UUserWidget> that holds a reference to a Blueprint class
	 *   derived from ULoadScreenWidget (this Blueprint class is set in the Editor on the Blueprint derived from ALoadScreenHUD)
	 * - This allows us to instantiate a Blueprint widget (with all its visual design and Blueprint logic) while still
	 *   having a strongly-typed C++ pointer (ULoadScreenWidget*) that we can use to call C++ functions
	 * - The Blueprint class adds visual elements and bindings, while the C++ class provides the programmatic interface
	 */
	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);

	// Add the load screen widget to the player's viewport to make it visible
	LoadScreenWidget->AddToViewport();

	// Call initialization logic to complete widget setup (e.g., view model binding)
	LoadScreenWidget->BlueprintInitializeWidget();
	
	// Loads saved data from disk for all save slots and updates their view models with player names and slot statuses
	LoadScreenViewModel->LoadData();
}
