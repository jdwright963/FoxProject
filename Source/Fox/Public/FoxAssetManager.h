// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "FoxAssetManager.generated.h"

/**
 * @class UFoxAssetManager
 * @brief A custom Asset Manager for managing game assets in the Fox project.
 *
 * The UFoxAssetManager class extends the functionality of the base UAssetManager
 * and is responsible for handling the initialization and global management of
 * game assets in the project.
 */
UCLASS()
class FOX_API UFoxAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	
	// Get singleton instance of this class
	static UFoxAssetManager& Get();
	
protected:
	
	// Called very early on and this is when we start the initial loading for our game assets
	virtual void StartInitialLoading() override;
};
