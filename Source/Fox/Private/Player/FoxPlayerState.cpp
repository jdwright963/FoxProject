// Copyright TryingToMakeGames


#include "Player/FoxPlayerState.h"

#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "Net/UnrealNetwork.h"

AFoxPlayerState::AFoxPlayerState()
{
	// Creates and initializes the Ability System Component as a subobject of this PlayerState.
	// This component manages gameplay abilities, attributes, and gameplay effects for the player.
	// "AbilitySystemComponent" is the subobject name identifier used for debugging and editor display.
	AbilitySystemComponent = CreateDefaultSubobject<UFoxAbilitySystemComponent>("AbilitySystemComponent");

	// Marks the ASC for replication so it synchronizes between server and clients in multiplayer games.
	AbilitySystemComponent->SetIsReplicated(true);

	// Sets the replication mode to Mixed: Gameplay Effects replicate to all clients, but Gameplay Cues 
	// and Gameplay Tags only replicate to the owning client. This balances network efficiency with gameplay needs.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Creates and initializes the Attribute Set that holds gameplay attributes (e.g., Health, Mana, Strength).
	// AttributeSet is automatically replicated through the ASC.
	// "AttributeSet" is the subobject name identifier used for debugging and editor display.
	AttributeSet = CreateDefaultSubobject<UFoxAttributeSet>("AttributeSet");
	
	// After tutorial was made Geter and seter functions were made in the engine code and the NetUpdateFrequency variable 
	// must be accessed with them
	// This sets how often (times per second) this PlayerState replicates to clients. Higher frequency ensures player stats 
	// like XP and Level are synchronized more quickly across the network, at the cost of increased bandwidth usage.
	SetNetUpdateFrequency(100.f);
}

void AFoxPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Registers the Level variable for replication to all clients. This macro tells the networking system
	// to automatically synchronize the Level value from the server to clients whenever it changes.
	DOREPLIFETIME(AFoxPlayerState, Level);

	// Registers the XP variable for replication to all clients. This macro tells the networking system
	// to automatically synchronize the XP value from the server to clients whenever it changes.
	DOREPLIFETIME(AFoxPlayerState, XP);
	
	// Registers the AttributePoints variable for replication to all clients. This macro tells the networking system
	// to automatically synchronize the XP value from the server to clients whenever it changes.
	DOREPLIFETIME(AFoxPlayerState, AttributePoints);
	
	// Registers the SpellPoints variable for replication to all clients. This macro tells the networking system
	// to automatically synchronize the XP value from the server to clients whenever it changes.
	DOREPLIFETIME(AFoxPlayerState, SpellPoints);
}

UAbilitySystemComponent* AFoxPlayerState::GetAbilitySystemComponent() const
{
	// This function returns the Ability System Component associated with this PlayerState.
	return AbilitySystemComponent;
}

void AFoxPlayerState::AddToXP(int32 InXP)
{
	// Increments the player's current XP by the specified amount (InXP).
	XP += InXP;

	// Broadcasts the updated XP value to all listeners subscribed to OnXPChangedDelegate.
	OnXPChangedDelegate.Broadcast(XP);
}

void AFoxPlayerState::AddToLevel(int32 InLevel)
{
	
	// Increments the player's current Level by the specified amount (InLevel).
	Level += InLevel;

	// Broadcasts the updated Level value to all listeners subscribed to OnLevelChangedDelegate.
	OnLevelChangedDelegate.Broadcast(Level);
}

void AFoxPlayerState::SetXP(int32 InXP)
{
	// Sets the player's current XP to the specified value (InXP), replacing the previous XP value entirely.
	XP = InXP;

	// Broadcasts the updated XP value to all listeners subscribed to OnXPChangedDelegate.
	OnXPChangedDelegate.Broadcast(XP);
}

void AFoxPlayerState::SetLevel(int32 InLevel)
{
	// Sets the player's current Level to the specified value (InLevel), replacing the previous Level value entirely.
	Level = InLevel;

	// Broadcasts the updated Level value to all listeners subscribed to OnLevelChangedDelegate.
	OnLevelChangedDelegate.Broadcast(Level);
}

void AFoxPlayerState::OnRep_Level(int32 OldLevel)
{
	// This replication notification function is automatically called on clients when the Level variable changes on the server.
	// It broadcasts the new Level value to all listeners subscribed to OnLevelChangedDelegate, ensuring UI and gameplay systems
	// are updated in response to the server-replicated Level change.
	OnLevelChangedDelegate.Broadcast(Level);
}

void AFoxPlayerState::OnRep_XP(int32 OldXP)
{
	// This replication notification function is automatically called on clients when the XP variable changes on the server.
	// It broadcasts the new XP value to all listeners subscribed to OnXPChangedDelegate, ensuring UI and gameplay systems
	// are updated in response to the server-replicated XP change.
	OnXPChangedDelegate.Broadcast(XP);
}

void AFoxPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	// This replication notification function is automatically called on clients when the AttributePoints variable changes on the server.
	// It broadcasts the new AttributePoints value to all listeners subscribed to OnAttributePointsChangedDelegate, ensuring UI and gameplay systems
	// are updated in response to the server-replicated AttributePoints change.
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AFoxPlayerState::OnRep_SpellPoints(int32 OldSpellPoints)
{
	// This replication notification function is automatically called on clients when the SpellPoints variable changes on the server.
	// It broadcasts the new SpellPoints value to all listeners subscribed to OnSpellPointsChangedDelegate, ensuring UI and gameplay systems
	// are updated in response to the server-replicated SpellPoints change.
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void AFoxPlayerState::AddToAttributePoints(int32 InPoints)
{
	// Increments the player's current AttributePoints by the specified amount (InPoints).
	AttributePoints += InPoints;

	// Broadcasts the updated AttributePoints value to all listeners subscribed to OnAttributePointsChangedDelegate.
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void AFoxPlayerState::AddToSpellPoints(int32 InPoints)
{
	// Increments the player's current SpellPoints by the specified amount (InPoints).
	SpellPoints += InPoints;

	// Broadcasts the updated SpellPoints value to all listeners subscribed to OnSpellPointsChangedDelegate.
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
