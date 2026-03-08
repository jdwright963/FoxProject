// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "FoxPlayerState.generated.h"


// Forward declarations
class ULevelUpInfo;
class UAbilitySystemComponent;
class UAttributeSet;

// Declares a multicast delegate type named FOnPlayerStatChanged that can broadcast to multiple listeners.
// This delegate takes one parameter: an int32 value representing a player stat (like XP or Level).
// Multiple objects can bind (subscribe) to this delegate to be notified when player stats change.
// The delegate is used to notify UI elements and gameplay systems about stat changes
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)

/**
 * 
 */
UCLASS()
class FOX_API AFoxPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:

	// Constructor that initializes the AFoxPlayerState object. This function creates and configures
	// the Ability System Component (ASC) and Attribute Set subobjects, sets up replication settings,
	// and configures the network update frequency for this PlayerState.
	AFoxPlayerState();

	// Override of APlayerState's GetLifetimeReplicatedProps function. This function is called by Unreal's
	// networking system to register which variables should be replicated from the server to clients.
	// We use this to register our Level and XP variables so they automatically synchronize across the network.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Implementation of IAbilitySystemInterface's GetAbilitySystemComponent function. This function provides
	// access to this PlayerState's Ability System Component, allowing other gameplay systems to interact with
	// the player's abilities, attributes, and gameplay effects.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Inline getter function that returns a pointer to the AttributeSet owned by this PlayerState.
	// This provides access to the player's gameplay attributes (Health, Mana, Strength, etc.) for
	// reading or modifying attribute values from other gameplay systems.
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	// Variable storing an instance of ULevelUpInfo (defined in AbilitySystem/Data/LevelUpInfo.h), which is a data asset
	// that defines level progression configuration for this player. This asset contains information about
	// XP requirements for each level, attribute rewards per level, and other level-up related data.
	// The value of this variable is set in the editor
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;
	
	// Multicast delegate that broadcasts XP change events to subscribed listeners. Other systems (such as widget controllers)
	// can bind functions to this delegate to be automatically notified whenever the player's
	// XP changes, receiving the new XP value as a parameter.
	FOnPlayerStatChanged OnXPChangedDelegate;

	// Multicast delegate that broadcasts Level change events to subscribed listeners. Other systems (such as widget controllers)
	// can bind functions to this delegate to be automatically notified whenever the player's
	// Level changes, receiving the new Level value as a parameter.
	FOnPlayerStatChanged OnLevelChangedDelegate;
	
	// Multicast delegate that broadcasts attribute points change events to subscribed listeners. Other systems (such as 
	// AttributeMenuWidgetController) can bind functions to this delegate to be automatically notified whenever the player's
	// available attribute points change, receiving the new attribute points value as a parameter. This allows the UI to
	// update the displayed available points for spending on primary attributes (Strength, Intelligence, Resilience, Vigor).
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;

	// Multicast delegate that broadcasts spell points change events to subscribed listeners. Other systems (such as 
	// SpellMenuWidgetController) can bind functions to this delegate to be automatically notified whenever the player's
	// available spell points change, receiving the new spell points value as a parameter. This allows the UI to
	// update the displayed available points for unlocking or upgrading abilities and spells.
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;
	
	// Returns the player's current level value.
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }

	// Returns the player's current experience points (XP) value.
	FORCEINLINE int32 GetXP() const { return XP; }
	
	// Returns the player's current attribute points available for spending on primary attributes.
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }

	// Returns the player's current spell points available for spending on abilities and spells.
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }

	// Increments the player's XP by the specified amount and broadcasts the change via OnXPChangedDelegate.
	void AddToXP(int32 InXP);

	// Increments the player's level by the specified amount and broadcasts the change via OnLevelChangedDelegate.
	void AddToLevel(int32 InLevel);
	
	// Increments the player's attribute points by the specified amount and broadcasts the change via OnAttributePointsChangedDelegate.
	// Attribute points are used to upgrade primary attributes (Strength, Intelligence, Resilience, Vigor) through the attribute menu UI.
	// The AttributeMenuWidgetController listens to OnAttributePointsChangedDelegate to update the displayed available points for spending.
	void AddToAttributePoints(int32 InPoints);

	// Increments the player's spell points by the specified amount and broadcasts the change via OnSpellPointsChangedDelegate.
	// Spell points are used to unlock or upgrade abilities and spells through the spell menu UI. The AttributeMenuWidgetController
	// listens to OnSpellPointsChangedDelegate to update the displayed available points for ability progression.
	void AddToSpellPoints(int32 InPoints);

	// Directly sets the player's XP to the specified value (replacing previous value) and broadcasts the change via OnXPChangedDelegate.
	void SetXP(int32 InXP);

	// Directly sets the player's level to the specified value (replacing previous value) and broadcasts the change via OnLevelChangedDelegate.
	void SetLevel(int32 InLevel);
	
protected:
	
	// These variables (AbilitySystemComponent and AttributeSet) are present in both AFoxPlayerState and 
	// AFoxCharacterBase because of different architectural patterns for player vs enemy characters:
	//
	// For ENEMIES (e.g., AFoxEnemy):
	// - AFoxEnemy inherits from AFoxCharacterBase
	// - It inherits these variables from the base class and constructs them in its constructor
	// - The ASC and AttributeSet live directly on the enemy character actor
	//
	// For PLAYER CHARACTERS:
	// - We place the ASC and AttributeSet on AFoxPlayerState instead of on the character
	// - This is a common multiplayer pattern because PlayerState persists across respawns and level transitions
	// - AFoxPlayerState does NOT inherit from AFoxCharacterBase (it inherits from APlayerState)
	// - Therefore, we cannot inherit these variables from AFoxCharacterBase
	// - We must define the same variables again in AFoxPlayerState to store the player's ASC and attributes
	//
	// Both classes implement IAbilitySystemInterface to provide access to their respective ASCs.
	
	// The Ability System Component (ASC) manages gameplay abilities, attributes, and gameplay effects for the player.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// The Attribute Set holds the player's gameplay attributes (e.g., Health, Mana, Strength, etc.)
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
private:
	
	// The player's current level. This value is replicated from the server to all clients.
	// When the server changes this value, the OnRep_Level function is automatically 
	// called on clients to handle the replication notification and broadcast the change via OnLevelChangedDelegate.
	// Initialized to 1 as the starting level for new players.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
	
	// The player's current experience points (XP). This value is replicated from the server to all clients.
	// When the server changes this value, the OnRep_XP function is automatically called on
	// clients to handle the replication notification and broadcast the change via OnXPChangedDelegate.
	// Initialized to 1 as the starting XP for new players.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
	int32 XP = 1;
	
	// The player's current attribute points available for spending. These points are used to upgrade
	// primary attributes (Strength, Intelligence, Resilience, Vigor) through the attribute menu UI.
	// This value is replicated from the server to all clients. When the server changes this value,
	// the OnRep_AttributePoints function is automatically called on clients. Is broadcast
	// the change via OnAttributePointsChangedDelegate, which notifies the
	// AttributeMenuWidgetController to update the UI display of available points.
	// Initialized to 0 as new players start without any attribute points to spend.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 0;

	// The player's current spell points available for spending. These points are used to unlock or
	// upgrade abilities and spells through the spell menu UI. This value is replicated from the
	// server to all clients. When the server changes this value, the OnRep_SpellPoints function is
	// automatically called on clients. It broadcasts the change via OnSpellPointsChangedDelegate, ensuring UI and 
	// gameplay systems react to spell point changes. Initialized to 0 to give new players start with 0 spell points.
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SpellPoints)
	int32 SpellPoints = 0;
	
	// Replication notification callback automatically invoked on clients when the Level variable changes on the server.
	// This function receives the previous Level value as a parameter (OldLevel) and broadcasts the new Level value
	// to all subscribed listeners via OnLevelChangedDelegate, ensuring UI and gameplay systems react to level changes.
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
	
	// Replication notification callback automatically invoked on clients when the XP variable changes on the server.
	// This function receives the previous XP value as a parameter (OldXP) and broadcasts the new XP value
	// to all subscribed listeners via OnXPChangedDelegate, ensuring UI and gameplay systems react to XP changes.
	UFUNCTION()
	void OnRep_XP(int32 OldXP);
	
	// Replication notification callback automatically invoked on clients when the AttributePoints variable changes on the server.
	// This function receives the previous AttributePoints value as a parameter (OldAttributePoints) and broadcasts the new
	// AttributePoints value to all subscribed listeners via OnAttributePointsChangedDelegate, ensuring UI systems (particularly
	// the AttributeMenuWidgetController) react to attribute point changes and update the displayed available points for spending.
	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	// Replication notification callback automatically invoked on clients when the SpellPoints variable changes on the server.
	// This function receives the previous SpellPoints value as a parameter (OldSpellPoints) and broadcasts the new SpellPoints
	// value to all subscribed listeners via OnSpellPointsChangedDelegate, ensuring UI systems (particularly
	// the AttributeMenuWidgetController) react to spell point changes and update the displayed available points for 
	// unlocking or upgrading abilities.
	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);
};
