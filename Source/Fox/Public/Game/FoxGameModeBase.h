// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FoxGameModeBase.generated.h"

class UAbilityInfo;
class UCharacterClassInfo;

/**
 * 
 */
UCLASS()
class FOX_API AFoxGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	// Variable to store the UCharacterClassInfo data asset. This is set in the editor
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
	
	// Variable to store the UAbilityInfo data asset. This is set in the editor and contains configuration data for all
	// abilities in the game, including their tags, icons, materials, level requirements, and ability classes
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;
};
