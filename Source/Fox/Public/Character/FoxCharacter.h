// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "Character/FoxCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "FoxCharacter.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;

/**
 * 
 */
UCLASS()
class FOX_API AFoxCharacter : public AFoxCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	
	// Constructor for this class
	AFoxCharacter();	
	
	/**
	 * PossessedBy - Server-side initialization when controller possesses this character
	 * 
	 * This function is called on the SERVER when a controller takes possession of this character.
	 * It is NOT called on clients. This is the ideal place to initialize server-side systems
	 * like the Ability System Component (ASC) for the player character.
	 * 
	 * Flow: Server creates character -> Server assigns controller -> PossessedBy called on server
	 * 
	 * @param NewController The controller that is taking possession of this character
	 */
	virtual void PossessedBy(AController* NewController) override;

	/**
	 * OnRep_PlayerState - Client-side callback when PlayerState is replicated
	 * 
	 * This function is called on the CLIENT when the PlayerState is replicated from the server.
	 * It is NOT called on the server. This is the ideal place to initialize client-side systems
	 * like the Ability System Component (ASC) for the player character on the owning client.
	 * 
	 * Flow: Server replicates PlayerState -> OnRep_PlayerState called on client
	 * 
	 * Together with PossessedBy(), these two functions ensure that both server and client
	 * properly initialize the character's ability system in a multiplayer environment.
	 */
	virtual void OnRep_PlayerState() override;
	
	/** Player Interface */
	
	/**
	 * AddToXP_Implementation - Implementation of the IPlayerInterface::AddToXP function
	 * 
	 * This function serves as a middleman between the Ability System (AttributeSet) and the PlayerState.
	 * When the player gains XP (e.g., from killing an enemy), the AttributeSet calls this function through
	 * the IPlayerInterface interface. This implementation then forwards the XP gain to the PlayerState (by calling a 
	 * different function, PlayerState->AddToXP()), which handles the actual XP accumulation, level-up logic, and UI updates.
	 * 
	 * WHY THIS MIDDLEMAN PATTERN?
	 * - Avoids circular dependency between PlayerState and AttributeSet
	 * - PlayerState owns AttributeSet (needs to include AttributeSet.h)
	 * - AttributeSet needs to communicate XP changes back to PlayerState
	 * - Direct communication would require AttributeSet to include PlayerState.h (circular dependency)
	 * - Interface breaks the cycle: PlayerState -> AttributeSet -> Interface -> Character -> PlayerState
	 * 
	 * FLOW:
	 * 1. Enemy dies and grants XP
	 * 2. AttributeSet detects XP attribute change
	 * 3. AttributeSet calls AddToXP() on the character (through IPlayerInterface)
	 * 4. This function (AddToXP_Implementation) receives the call
	 * 5. This function forwards the XP to PlayerState->AddToXP() (a different AddToXP() function)
	 * 6. PlayerState handles XP accumulation, level-up checks, and broadcasts updates
	 * 
	 * Note: This is the C++ implementation of a BlueprintNativeEvent. Blueprint can override this,
	 * but if not overridden, this C++ version executes.
	 * 
	 * @param InXP The amount of experience points to add to the player
	 */
	virtual void AddToXP_Implementation(int32 InXP) override;
	
	// Function to handle character leveling up this function is delcared in PlayerInterface.h and overriden here
	virtual void LevelUp_Implementation() override;
	
	// Function to get the characters current XP this function is delcared in PlayerInterface.h and overriden here
	virtual int32 GetXP_Implementation() const override;
	
	// Function to find the level for a given XP amount this is declared in PlayerInterface.h and overriden here
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	
	// Function to get the characters attribute points reward for a given level this function is declared in
	// PlayerInterface.h and overriden here
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	
	// Function to get the characters spell points reward for a given level this function is declared in 
	// PlayerInterface.h and overriden here
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	
	// Function to increase the player's level by the specified amount this function is declared in 
	// PlayerInterface.h and overriden here
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	
	// Function to add the specified amount of attribute points to the character this function is declared in 
	// PlayerInterface.h and overriden here
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	
	// Function to add the specified amount of spell points to the character this function is declared in 
	// PlayerInterface.h and overriden here
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	
	// Function to get the character's current attribute points this function is declared in 
	// PlayerInterface.h and overriden here
	virtual int32 GetAttributePoints_Implementation() const override;
	
	// Function to get the character's current spell points this function is declared in 
	// PlayerInterface.h and overriden here
	virtual int32 GetSpellPoints_Implementation() const override;
	
	/** end Player Interface */
	
	/** Combat Interface */
	
	// Function to get the Level of this character. This function is declared in PlayerInterface.h and overriden here
	virtual int32 GetPlayerLevel_Implementation() override;
	
	/** end Combat Interface */
	
	/**
	 * LevelUpNiagaraComponent - Visual effect component for level-up celebration
	 * 
	 * This Niagara particle system component is attached to the character and plays
	 * a visual effect when the player levels up. It is activated by the MulticastLevelUpParticles()
	 * function, which is called on all clients when a level-up occurs.
	 * 
	 * VisibleAnywhere: Allows viewing this component in the editor and at runtime
	 * BlueprintReadOnly: Can be read by Blueprint scripts but not modified
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;
	
	/**
	 * OnRep_Stunned - Declared in FoxCharacterBase.h and overridden here
	 * 
	 * This is an OnRep function that is called only on CLIENTS when the bIsStunned variable
	 * is replicated from the server. Updates visual effects like stun particle effects and
	 * enables or disables the characters ability to move or interact with the world depending on the value of bIsStunned
	 */
	virtual void OnRep_Stunned() override;

	/**
	 * OnRep_Burned - Declared in FoxCharacterBase.h and overridden here
	 * 
	 * This is an OnRep function that is called only on CLIENTS when the bIsBurned variable
	 * is replicated from the server. It enables or disables the fire particle effect depending on the value of bIsBurned
	 */
	virtual void OnRep_Burned() override;
	
private:
	
	/**
	 * TopDownCameraComponent - The camera that provides the top-down view of the game world
	 * 
	 * This camera component is attached to the CameraBoom (spring arm) and provides the player's
	 * view of the game world from above. In a top-down game, this camera looks down at the character
	 * from a fixed angle, allowing the player to see the surrounding environment and plan their actions.
	 * 
	 * The camera's position and rotation are determined by the CameraBoom component, which handles
	 * distance from the character and can provide smooth camera movement or collision handling.
	 * 
	 * VisibleAnywhere: Allows viewing and selecting this component in the editor for positioning adjustments
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/**
	 * CameraBoom - Spring arm component that positions the camera at a distance from the character
	 * 
	 * This spring arm (USpringArmComponent) acts as a flexible boom that holds the TopDownCameraComponent
	 * at a specified distance and angle from the character. It provides several important features:
	 * 
	 * KEY FEATURES:
	 * - Distance Control: Maintains a fixed distance between the character and camera
	 * - Smooth Movement: Can interpolate camera position for smooth following behavior
	 * - Collision Handling: Can pull the camera closer if the boom collides with world geometry,
	 *   preventing the camera from clipping through walls or objects
	 * - Rotation Control: Defines the angle at which the camera looks at the character (e.g., 45-degree angle for top-down)
	 * 
	 * VisibleAnywhere: Allows viewing and adjusting this component in the editor to fine-tune camera positioning
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	/**
	 * InitAbilityActorInfo - Initialize Ability System Component actor information
	 * 
	 * This function initializes the Ability System Component (ASC) by setting up the connection
	 * between the ASC, the owner actor (the PlayerState), and the avatar actor (this character).
	 * It is called from both PossessedBy() on the server and OnRep_PlayerState() on the client
	 * to ensure proper initialization in multiplayer environments.
	 * 
	 * WHY INITIALIZE ON BOTH SERVER AND CLIENT?
	 * 
	 * In Unreal Engine's client-server architecture, each machine (server and clients) runs its own
	 * instance of the game simulation. The Gameplay Ability System needs to function on both:
	 * 
	 * SERVER INITIALIZATION (via PossessedBy):
	 * - The server is the authority and makes all gameplay decisions (ability activation, damage calculation, etc.)
	 * - When a controller possesses a character, PossessedBy() is called ONLY on the server
	 * - We initialize the ASC here so the server can immediately begin processing ability logic
	 * - The server needs the ASC initialized to grant abilities, apply effects, and respond to ability requests
	 * 
	 * CLIENT INITIALIZATION (via OnRep_PlayerState):
	 * - Clients need their own local copy of the ASC to predict gameplay, show UI, and play cosmetic effects
	 * - When the server replicates the PlayerState to a client, OnRep_PlayerState() is called ONLY on that client
	 * - We initialize the ASC here so the client can display ability cooldowns, costs, and other UI elements
	 * - Client-side initialization enables ability prediction (starting abilities locally before server confirms)
	 * - Without client initialization, the player would see no abilities in their UI and have no visual feedback
	 * 
	 * EXAMPLE FLOW IN MULTIPLAYER:
	 * 1. Server creates character and assigns controller -> PossessedBy() called on server -> Server ASC initialized
	 * 2. Server replicates PlayerState to owning client -> OnRep_PlayerState() called on client -> Client ASC initialized
	 * 3. Both machines now have properly initialized ASCs for their respective roles
	 * 
	 * For player characters this function typically:
	 * - Retrieves the ASC from the PlayerState
	 * - Calls InitAbilityActorInfo() (the ASC function with the same name as this function) on the ASC with owner 
	 *   (PlayerState) and avatar (this character) parameters
	 * - Initializes attributes and grants default abilities
	 * 
	 * Note: The OwnerActor is the PlayerState (which owns the ASC), and the AvatarActor is this character
	 * (the physical pawn in the world). This separation allows the ASC to persist on the PlayerState even
	 * when the character is destroyed and respawned.
	 * 
	 * This override allows the player character to have its own specific initialization logic
	 * that differs from the base character class implementation.
	*/
	virtual void InitAbilityActorInfo() override;
	
	
	/**
	 * MulticastLevelUpParticles - Replicate level-up particle effect to all clients
	 * 
	 * This function is called on the server when a player levels up and automatically replicates
	 * the call to all connected clients (including the server itself). It activates the
	 * LevelUpNiagaraComponent to play a visual celebration effect visible to all players.
	 * 
	 * NETWORKING BEHAVIOR:
	 * - NetMulticast: Function executes on the server and all clients when called on the server
	 * - Reliable: Guarantees the RPC will be delivered even if packets are lost (important for gameplay events)
	 * - If called on a client (non-authority), the function will only execute locally (no replication occurs)
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;
};
