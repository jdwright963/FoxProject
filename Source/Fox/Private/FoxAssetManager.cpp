// Copyright TryingToMakeGames


#include "FoxAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "FoxGameplayTags.h"

UFoxAssetManager& UFoxAssetManager::Get()
{
	// Get the singleton Asset Manager and cast it to UFoxAssetManager and return it.
	check(GEngine);
	UFoxAssetManager* FoxAssetManager = Cast<UFoxAssetManager>(GEngine->AssetManager);
	return *FoxAssetManager;
}

void UFoxAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	// Calls the function that adds native C++ gameplay tags to the UGameplayTagManager 
	FFoxGameplayTags::InitializeNativeGameplayTags();
	
	// Not sure if this is necessary, we do not get an error when using target data without this.
	// Supposedly it is required to use target data, but maybe that is old.
	UAbilitySystemGlobals::Get().InitGlobalData();
}
