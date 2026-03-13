// Copyright TryingToMakeGames


#include "Character/FoxCharacter.h"

#include "AbilitySystemComponent.h"
#include "FoxGameplayTags.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/FoxPlayerController.h"
#include "Player/FoxPlayerState.h"
#include "UI/HUD/FoxHUD.h"

AFoxCharacter::AFoxCharacter()
{
	
	/**
	 * Create the camera boom (spring arm) component for positioning the camera
	 * 
	 * USpringArmComponent acts as a movable arm that the camera attaches to. It provides:
	 * - Distance control between the camera and character (arm length)
	 * - Smooth camera following behavior with optional lag
	 * - Collision detection to prevent camera clipping through walls (disabled below)
	 * 
	 * For top-down games, the boom is typically positioned above and angled down at the character.
	 */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");

	/**
	 * Attach the camera boom to the character's root component
	 * 
	 * GetRootComponent() returns the character's capsule component (from ACharacter class)
	 * This makes the camera boom follow the character's position as they move through the world
	 */
	CameraBoom->SetupAttachment(GetRootComponent());

	/**
	 * Use absolute rotation instead of inheriting rotation from the parent component
	 * 
	 * When true:
	 * - The boom maintains a fixed world rotation (e.g., always pointing down from above)
	 * - Character rotation doesn't affect camera orientation
	 * - Perfect for top-down games where the camera should stay at a constant angle and not rotate with the character
	 * 
	 * When false (default):
	 * - The boom would rotate with the character, causing the camera to spin around
	 * - Useful for third-person cameras that follow behind the character's back
	 */
	CameraBoom->SetUsingAbsoluteRotation(true);

	/**
	 * Disable collision testing for the camera boom
	 * 
	 * By default, USpringArmComponent performs collision checks and pulls the camera closer
	 * when objects are between the camera and target (to prevent clipping through walls)
	 * 
	 * For top-down games:
	 * - Camera is positioned high above the scene looking down
	 * - Collision detection is unnecessary and could cause unwanted camera movement
	 * 
	 * For third-person games, you'd typically leave this as true (default)
	 */
	CameraBoom->bDoCollisionTest = false;

	/**
	 * Create the top-down camera component
	 * 
	 * UCameraComponent is the actual camera that renders the scene from the player's view
	 * It must be attached to something (usually a spring arm) to position it in the world
	 * The camera's location and rotation determine what the player sees on screen
	 */
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");

	/**
	 * Attach the camera to the end of the spring arm
	 * 
	 * USpringArmComponent::SocketName is a predefined socket at the tip of the spring arm
	 * This positions the camera at the boom's endpoint, looking toward the character
	 * The camera will follow the boom's position (but not the character's rotation
	 * due to SetUsingAbsoluteRotation above)
	 */
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	
	/**
	 * Disable pawn control rotation for the camera
	 * 
	 * When false (set here):
	 * - Camera rotation is NOT controlled by player controller input (mouse/gamepad look)
	 * - Camera maintains its fixed orientation set in the editor or through absolute rotation
	 * - Perfect for top-down games where the camera angle is fixed from above
	 * 
	 * When true (default for third-person):
	 * - Player controller input (mouse movement, right stick) rotates the camera
	 * - Allows free-look camera control for third-person or first-person games
	 * 
	 * Key difference from CameraBoom->SetUsingAbsoluteRotation (set above):
	 * - SetUsingAbsoluteRotation(true): Spring arm ignores PARENT rotation (character rotation doesn't affect boom)
	 *   * Boom stays at fixed world rotation even when character spins around
	 *   * Without this, when character rotates, the boom and camera would orbit around them
	 * 
	 * - bUsePawnControlRotation = false: Camera ignores CONTROLLER INPUT rotation (mouse look disabled)
	 *   * Player cannot manually rotate the camera with mouse/gamepad
	 *   * Without this, player could free-look and rotate the camera away from the top-down view
	 * 
	 * Both work together for top-down view:
	 * 1. SetUsingAbsoluteRotation keeps boom fixed when CHARACTER rotates (character spins, camera doesn't)
	 * 2. bUsePawnControlRotation = false prevents PLAYER INPUT from rotating camera (no mouse look)
	 * 
	 * Example without SetUsingAbsoluteRotation: Character turns right → boom orbits → camera circles around character
	 * Example without bUsePawnControlRotation: Player moves mouse → camera tilts/rotates → loses top-down angle
	 * 
	 * Note: This is also different from bUseControllerRotationYaw (disabled below)
	 * - bUsePawnControlRotation affects the camera component itself
	 * - bUseControllerRotationYaw affects the character's rotation
	 */
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	
	// Create a Niagara particle system component for playing visual effects when the character levels up
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");

	// Attach the level-up effect to the character's root component so it follows the character's position
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());

	// Disable automatic activation. The effect should only play when triggered manually during level-up events
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	// Make the character automatically rotate to face the direction of movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	/**
	 * Set rotation speed (yaw) to 400 degrees per second for smooth turning
	 * 
	 * RotationRate controls how fast the character rotates to face the movement direction when 
	 * bOrientRotationToMovement is true. The FRotator parameters represent (Pitch, Yaw, Roll):
	 * - Pitch (0.0f): No rotation around the Y-axis (looking up/down)
	 * - Yaw (400.0f): Rotation speed of 400 degrees per second around the Z-axis (turning left/right)
	 * - Roll (0.0f): No rotation around the X-axis (tilting side to side)
	 * 
	 * 400 degrees per second provides smooth, responsive turning without being instantaneous or too sluggish.
	 * This value can be adjusted based on gameplay feel - higher values create snappier turns, lower values 
	 * create more gradual, realistic rotation.
	 */
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);

	
	/**
	 * Constrain character movement to a single plane (for top-down gameplay)
	 * 
	 * Understanding VELOCITY vs POSITION:
	 * - POSITION: Where the character is located in the world (their X, Y, Z coordinates)
	 * - VELOCITY: How fast and in what direction the character is moving (rate of change of position)
	 * 
	 * What bConstrainToPlane does:
	 * - Constrains VELOCITY (movement direction/speed), NOT position (location)
	 * - The character's POSITION can still change in all directions (including up/down on stairs)
	 * - The character's VELOCITY is kept parallel to the ground plane (can't gain upward/downward momentum)
	 * 
	 * Practical example:
	 * WITHOUT this constraint: An explosion could give the character upward velocity, launching them into the air,
	 *                          and their position would keep going up as long as they have that upward velocity
	 * WITH this constraint:    The explosion can't give upward velocity - it's filtered out. The character's 
	 *                          position stays grounded because they never gain the velocity to leave the ground
	 * 
	 * Important: The character CAN still walk up stairs and slopes because the movement system directly changes
	 * their POSITION to follow the ground. This setting only prevents external forces from giving them VELOCITY
	 * that would make them float/fly away from surfaces.
	 */
	GetCharacterMovement()->bConstrainToPlane = true;

	/**
	 * Snap the character to the movement plane at the start of the game
	 * 
	 * What this actually does:
	 * - At game start, this removes any VELOCITY component that's perpendicular to the constraint plane
	 * - Does NOT change the character's POSITION (where they are spawned)
	 * - Ensures the character starts with zero upward/downward velocity
	 * 
	 * Why this matters:
	 * - If you spawn the character slightly above the ground, they might have downward velocity from falling
	 * - This setting removes that velocity immediately, preventing physics jitter on the first frame
	 * 
	 * What it does NOT do:
	 * - Does NOT teleport or move the character's position
	 * - Does NOT prevent the character from walking on uneven terrain after game starts
	 * - Only affects the initial velocity state, not ongoing position changes from walking
	 */
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
	/**
	 * Disable controller rotation inheritance for top-down gameplay
	 * 
	 * Understanding Controller Rotation vs Character Rotation:
	 * - CONTROLLER ROTATION: The direction the player's camera/view is looking (controlled by mouse/gamepad)
	 * - CHARACTER ROTATION: The direction the character mesh is facing in the world
	 * 
	 * What these settings do:
	 * - By default, characters automatically rotate to match their controller's rotation
	 * - Setting these to false DECOUPLES character rotation from controller rotation
	 * - The character can face one direction while the camera looks in another direction
	 * 
	 * Why disable for top-down games:
	 * - In top-down view, the camera looks down at the character from above (fixed pitch angle)
	 * - We don't want the character to tilt or rotate to match the camera's downward angle
	 * - Instead, we use bOrientRotationToMovement to make the character face their movement direction
	 * - This creates natural, intuitive movement where the character faces where they're walking
	 * 
	 * Axis Breakdown:
	 * - Pitch (false): Character won't tilt up/down to match camera's downward viewing angle
	 * - Roll (false): Character won't roll/tilt sideways based on controller input
	 * - Yaw (false): Character won't automatically turn left/right to match controller rotation
	 *                (rotation is handled by bOrientRotationToMovement instead)
	 */
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	/**
	 * Set default character class for player characters
	 * 
	 * ECharacterClass is a scoped enum defined in CharacterClassInfo.h that categorizes characters into
	 * gameplay classes (Elementalist, Warrior, Ranger). Each class has different:
	 * - Primary attributes (strength, intelligence, etc.)
	 * - Starting abilities
	 * - XP rewards (for enemies)
	 * 
	 * Elementalist is set as the default class for player characters. This value determines which
	 * FCharacterClassDefaultInfo entry is used from the CharacterClassInformation TMap when initializing
	 * the character's attributes and abilities.
	 * 
	 * Note: Enemy characters will have their CharacterClass set in their respective Blueprint classes.
	 */
	CharacterClass = ECharacterClass::Elementalist;
}

void AFoxCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// Init ability actor info for the server
	InitAbilityActorInfo();
	
	// This function calls the function in UFoxAbilitySystemComponent that grants the abilities to the owner of the ASC
	// passing it the StartupAbilities TSubclassOf TArray defined in AFoxCharacterBase
	AddCharacterAbilities();
}

void AFoxCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Init ability actor info for the server
	InitAbilityActorInfo();
}

void AFoxCharacter::AddToXP_Implementation(int32 InXP)
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);

	/**
	 * Forward the XP gain to the PlayerState's AddToXP method
	 * 
	 * This completes the XP flow chain:
	 * 1. AttributeSet detects XP attribute change
	 * 2. AttributeSet calls this function (AddToXP_Implementation) through IPlayerInterface
	 * 3. This function forwards the XP to PlayerState->AddToXP() (a different function with the same name)
	 * 4. PlayerState handles XP accumulation, checks for level-ups, and broadcasts UI updates
	 * 
	 * Note: This PlayerState->AddToXP() is a DIFFERENT function than this AddToXP_Implementation().
	 * We're calling AFoxPlayerState::AddToXP(), not AFoxCharacter::AddToXP_Implementation().
	 * 
	 * @param InXP The amount of experience points being forwarded to the PlayerState
	 */
	FoxPlayerState->AddToXP(InXP);
}

void AFoxCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

void AFoxCharacter::MulticastLevelUpParticles_Implementation() const
{
	
	/**
	 * Validate the Niagara component exists before attempting to use it
	 * 
	 * IsValid() checks if the pointer is:
	 * - Not nullptr (pointer points to a valid memory address)
	 * - Not pending kill (object hasn't been marked for garbage collection)
	 * - Not in the process of being destroyed
	 * 
	 * This check is necessary because:
	 * - The component could fail to initialize during construction
	 * - The component might have been destroyed during gameplay
	 * - Accessing an invalid pointer would cause a crash
	 */
	if (IsValid(LevelUpNiagaraComponent))
	{
		/**
		 * Get the camera's current world location
		 * 
		 * GetComponentLocation() returns an FVector containing the X, Y, Z coordinates
		 * of where the TopDownCameraComponent is positioned in world space (not relative to parent)
		 * 
		 * We need this position to calculate which direction the particle effect should face
		 * The particle effect will be rotated to look at this camera location
		 */
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();

		/**
		 * Get the Niagara particle system's current world location
		 * 
		 * GetComponentLocation() returns an FVector containing the X, Y, Z coordinates
		 * of where the LevelUpNiagaraComponent is attached (at the character's root)
		 * 
		 * This is the starting point for our rotation calculation.
		 */
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();

		/**
		 * Calculate the rotation needed to make the particle system face the camera
		 * 
		 * Breaking down this line:
		 * 1. (CameraLocation - NiagaraSystemLocation) - Vector subtraction creates a direction vector
		 *    - This vector points FROM the particle system TO the camera
		 *    - Example: Camera at (100, 200, 300), Particle at (50, 100, 150) = Direction (50, 100, 150)
		 * 
		 * 2. .Rotation() - Converts the direction vector into an FRotator (Pitch, Yaw, Roll angles)
		 *    - Calculates the angles needed to point in that direction
		 *    - This is the rotation that will make the particle effect face the camera
		 * 
		 * Why orient toward the camera:
		 * - Makes the particle effect more visually appealing by facing the player's view
		 * - Ensures the best viewing angle for the effect regardless of camera position
		 * - Particularly important for directional particle effects that have a "front" side
		 */
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();

		/**
		 * Apply the calculated rotation to the Niagara component in world space
		 * 
		 * SetWorldRotation() sets the component's absolute rotation in the world (not relative to parent)
		 * This makes the particle effect face toward the camera, ensuring optimal visual presentation
		 * 
		 * Note: We use world rotation instead of relative rotation because:
		 * - The component is attached to the character's root (which rotates as character moves)
		 * - World rotation ignores parent rotation, keeping the effect oriented toward the camera
		 * - If we used relative rotation, the effect would rotate with the character and lose camera facing
		 */
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);

		/**
		 * Activate the Niagara particle system to play the level-up visual effect
		 * 
		 * Activate(bool bReset) starts playing the particle system:
		 * - bReset = true: Resets the system to time 0 and restarts from the beginning
		 *   * Ensures a fresh, full playback of the effect every time
		 *   * Clears any previous state or ongoing playback
		 *   * Guarantees consistent visual presentation for each level-up
		 * 
		 * - bReset = false (alternative): Would continue from current state without restarting
		 *   * Could result in partial or incomplete effect playback if called mid-animation
		 * 
		 * The particle system will play once and automatically deactivate when complete
		 * (assuming bAutoActivate is false and the Niagara system is set to play once)
		 */
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 AFoxCharacter::GetXP_Implementation() const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	const AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Return the XP value from the PlayerState
	return FoxPlayerState->GetXP();
}

int32 AFoxCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	const AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Look up what level the player should be based on their total XP amount
	return FoxPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 AFoxCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	const AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	/**
	 * Retrieve the attribute point reward for the specified level
	 * 
	 * Breaking down this line:
	 * - FoxPlayerState->LevelUpInfo: Access the ULevelUpInfo data asset that stores level progression information
	 * - ->LevelUpInformation[Level]: Index into the TArray<FFoxLevelUpInfo> using the provided Level parameter
	 * - .AttributePointAward: Access the AttributePointAward member of the FFoxLevelUpInfo struct
	 * 
	 * What this returns:
	 * - The number of attribute points the player should receive when reaching the specified level
	 * - This value is defined in the BP_LevelUpInfo data asset in the editor
	 * - Attribute points can be spent to increase primary attributes (Strength, Intelligence, etc.)
	 * 
	 * Note: Array access assumes Level is a valid index (1-based indexing where Level 1 is at index 1)
	 * The LevelUpInformation array should be pre-populated with enough entries to cover all possible levels
	*/
	return FoxPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 AFoxCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	const AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	/**
	 * Retrieve the spell point reward for the specified level
	 * 
	 * Breaking down this line:
	 * - FoxPlayerState->LevelUpInfo: Access the ULevelUpInfo data asset that stores level progression information
	 * - ->LevelUpInformation[Level]: Index into the TArray<FFoxLevelUpInfo> using the provided Level parameter
	 * - .SpellPointAward: Access the SpellPointAward member of the FFoxLevelUpInfo struct
	 * 
	 * What this returns:
	 * - The number of spell points the player should receive when reaching the specified level
	 * - This value is defined in the BP_LevelUpInfo data asset in the editor
	 * - Spell points can be spent to unlock new abilities or upgrade existing ones
	 * 
	 * Note: Array access assumes Level is a valid index (1-based indexing where Level 1 is at index 1)
	 * The LevelUpInformation array should be pre-populated with enough entries to cover all possible levels
	*/
	return FoxPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void AFoxCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Use the function on FoxPlayerState to Increment the player's level by the specified amount and broadcasts the
	// change via OnLevelChangedDelegate.
	FoxPlayerState->AddToLevel(InPlayerLevel);
	
	/**
	 * Cast the Ability System Component to UFoxAbilitySystemComponent and perform null-check in one statement.
	 * GetAbilitySystemComponent() retrieves the UAbilitySystemComponent* pointer stored in this character
	 */
	if (UFoxAbilitySystemComponent* FoxASC = Cast<UFoxAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		/**
		 * Update the status of all abilities based on the new player level
		 * 
		 * UpdateAbilityStatuses() iterates through all granted abilities and updates their StatusTag
		 * (e.g., "Abilities.Status.Locked", "Abilities.Status.Eligible", "Abilities.Status.Unlocked")
		 * based on the current player level and ability requirements.
		 * 
		 * Why this is called here:
		 * - When a player levels up, previously locked abilities may now meet their level requirements
		 * - This function evaluates each ability's LevelRequirement against the new player level
		 * - Status changes trigger AbilityStatusChanged delegate, updating UI to show newly available abilities
		 * 
		 * @param FoxPlayerState->GetPlayerLevel() - The character's new level after leveling up
		 */
		FoxASC->UpdateAbilityStatuses(FoxPlayerState->GetPlayerLevel());
	}
}

void AFoxCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Forward the attribute points to the PlayerState's AddToAttributePoints function
	FoxPlayerState->AddToAttributePoints(InAttributePoints);
}

void AFoxCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Forward the spell points to the PlayerState's AddToSpellPoints function
	FoxPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 AFoxCharacter::GetAttributePoints_Implementation() const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Return the player's current unspent attribute points from the PlayerState
	return FoxPlayerState->GetAttributePoints();
}

int32 AFoxCharacter::GetSpellPoints_Implementation() const
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Return the player's current unspent spell points from the PlayerState
	return FoxPlayerState->GetSpellPoints();
}

int32 AFoxCharacter::GetPlayerLevel_Implementation()
{
	/**
	 * Retrieve and cast the PlayerState to AFoxPlayerState
	 * 
	 * GetPlayerState<AFoxPlayerState>() is a templated function that:
	 * - Gets the APlayerState associated with this character
	 * - Performs a type-safe cast to AFoxPlayerState (our custom PlayerState class)
	 * - Returns nullptr if the cast fails or if there is no PlayerState
	 */
	const AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();

	/**
	 * Validate that PlayerState exists using check() macro
	 * 
	 * check() is an Unreal assertion macro that:
	 * - Verifies the pointer is not nullptr
	 * - If nullptr, crashes the game in Development builds with a meaningful error message
	 * - Helps catch bugs during development where PlayerState is unexpectedly missing
	 * - Is compiled out in Shipping builds for performance (unlike ensure())
	 */
	check(FoxPlayerState);
	
	// Return the player's current level from the PlayerState 
	return FoxPlayerState->GetPlayerLevel();
}

void AFoxCharacter::OnRep_Stunned()
{
	// Cast the inherited Ability System Component to UFoxAbilitySystemComponent, store it in a variable, and perform a
	// null-check to see if the cast was successful
	if (UFoxAbilitySystemComponent* FoxASC = Cast<UFoxAbilitySystemComponent>(AbilitySystemComponent))
	{
		/**
		 * Retrieve the singleton instance of FFoxGameplayTags
		 * 
		 * FFoxGameplayTags::Get() returns a const reference to the singleton that holds all gameplay tags used in the project
		 * This provides centralized access to tags without repeatedly calling FGameplayTag::RequestGameplayTag().
		 * We store this in a const reference to avoid copying the entire tags structure
		 */
		const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

		/**
		 * Create a container to hold all input-blocking gameplay tags
		 * 
		 * FGameplayTagContainer is a container that can hold multiple FGameplayTag instances
		 * We use this to group all player input blocking tags together so they can be added/removed as a batch
		 * 
		 * This is more efficient than adding/removing tags individually and ensures all blocking tags are applied atomically
		 * When stunned, since all four input types (cursor trace, held, pressed, released) should be blocked simultaneously
		 */
		FGameplayTagContainer BlockedTags;

		/**
		 * Add Player_Block_CursorTrace tag to disable mouse cursor interaction while stunned
		 * 
		 * Player_Block_CursorTrace prevents the player from:
		 * - Hovering over enemies to highlight them
		 * - Clicking on targets to select them
		 * - Using mouse cursor for any combat targeting
		 * 
		 * This is crucial for stun mechanics because a stunned character should not be able to interact with the world
		 * The tag is checked in AFoxPlayerController's cursor trace logic to skip all mouse interaction when present
		 */
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);

		/**
		 * Add Player_Block_InputHeld tag to disable continuous held input detection while stunned
		 * 
		 * Player_Block_InputHeld prevents the player from:
		 * - Holding down movement keys (WASD) to continuously move
		 * - Holding ability buttons to charge or continuously cast
		 * - Any input that registers as "being held down"
		 * 
		 * This is essential for stun mechanics because it prevents the character from moving or casting while disabled
		 * The tag is checked in input handling code to ignore InputHeld events when present
		 */
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);

		/**
		 * Add Player_Block_InputPressed tag to disable initial button press detection while stunned
		 * 
		 * Player_Block_InputPressed prevents the player from:
		 * - Pressing movement keys to initiate movement
		 * - Pressing ability keys to start casting
		 * - Any input that registers as "pressed"
		 * 
		 * This prevents players from performing actions or abilities while stunned
		 * The tag is checked in input handling code to ignore InputPressed events when present
		 */
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);

		/**
		 * Add Player_Block_InputReleased tag to disable button release detection while stunned
		 * 
		 * Player_Block_InputReleased prevents the player from:
		 * - Releasing ability keys to complete charged abilities
		 * - Any input that registers as "just released" this frame
		 * 
		 * This ensures abilities that activate on release (like charged shots) cannot be triggered while stunned
		 * The tag is checked in input handling code to ignore InputReleased events when present
		 */
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		
		/**
		 * Check if the character is currently stunned
		 * 
		 * bIsStunned is a replicated boolean variable (defined in AFoxCharacterBase) that indicates whether the 
		 * character is stunned. This variable is set to true when a stun debuff is applied and false when it's removed.
		 * This function OnRep_Stunned() is automatically called on clients when bIsStunned changes due to replication
		 * 
		 * These if/else branches handle both applying and removing stun effects:
		 * - true: Character just became stunned → block input and show visual effect
		 * - false: Stun effect ended → restore input and hide visual effect
		 */
		if (bIsStunned)
		{
			/**
			 * Apply all input-blocking tags to the Ability System Component.
			 *
			 * "Loose" tags are Gameplay Tags managed directly by the ASC without the overhead 
			 * of a Gameplay Effect (GE). Unlike tags granted by a GE, which are tied to the 
			 * effect's duration, Loose tags are manually added and removed via C++. 
			 * 
			 * These tags are "Reference Counted." If multiple overlapping systems (e.g., a Stun 
			 * and a Hit-Reaction) add the same blocking tag, the tag will only be removed from 
			 * the ASC once every system has called for its removal. This ensures that the 
			 * player remains locked out of input as long as at least one blocking source exists.
			 *
			 * Effect on gameplay:
			 * - PlayerController checks the ASC for these specific tags before processing input.
			 * - When any block tag is present, the corresponding movement or ability input is ignored.
			 * - This creates a fail-safe input lockout during stun/hit-react states.
			 *
			 * Why use tags instead of boolean flags:
			 * - Tags integrate natively with GAS's Ability Activation requirements (e.g., "Block Abilities").
			 * - Reference counting handles overlapping status effects automatically without complex bool logic.
			 * - Systems remain decoupled, so any new system can check these tags without knowing about the Stun system.
			 */
			FoxASC->AddLooseGameplayTags(BlockedTags);

			/**
			 * Activate the stun visual effect
			 * 
			 * StunDebuffComponent is a UDebuffNiagaraComponent (defined in AFoxCharacterBase) that displays
			 * a Niagara particle system to visually indicate the character is stunned
			 * 
			 * Activate() starts playing the particle system:
			 * - Spawns stun effect particles around the character (e.g., stars circling head, electric sparks)
			 * - Provides visual feedback to players that the character cannot act
			 * - Helps identify stunned enemies in combat for tactical decision-making
			 * 
			 * The component remains active until Deactivate() is called when the stun ends
			 * This creates a persistent visual indicator for the entire duration of the stun effect
			 */
			StunDebuffComponent->Activate();
		}
		else
		{
			/**
			 * Remove all input-blocking tags from the Ability System Component
			 * 
			 * RemoveLooseGameplayTags() removes the entire BlockedTags container from the ASC's active tags
			 * This is the inverse operation of AddLooseGameplayTags() called when the stun was applied
			 * 
			 * What this does:
			 * - Removes Player_Block_CursorTrace tag → restores mouse cursor interaction
			 * - Removes Player_Block_InputHeld tag → allows continuous held input again
			 * - Removes Player_Block_InputPressed tag → enables new button presses
			 * - Removes Player_Block_InputReleased tag → processes button releases normally
			 * 
			 * Effect on gameplay:
			 * - Player controller no longer sees blocking tags and resumes processing input
			 * - Character can move, cast abilities, and interact with the world again
			 * - Returns full control to the player after stun duration expires
			 * 
			 * Tag counting safety:
			 * - If multiple stuns were applied, tags only fully remove when count reaches 0
			 * - This prevents premature input restoration if overlapping stuns exist
			 * - GAS automatically handles the reference counting for loose tags
			 */
			FoxASC->RemoveLooseGameplayTags(BlockedTags);

			/**
			 * Deactivate the stun visual effect
			 * 
			 * StunDebuffComponent is a UDebuffNiagaraComponent that was activated when the stun began
			 * 
			 * Deactivate() stops playing the particle system:
			 * - Removes the visual stun effect from the character
			 * - Provides clear visual feedback that the stun has ended
			 * - Signals to players that the character has regained control
			 */
			StunDebuffComponent->Deactivate();
		}
	}
}

void AFoxCharacter::OnRep_Burned()
{
	/**
	 * Check if the character has the burn debuff
	 * 
	 * bIsBurned is a replicated boolean variable (defined in AFoxCharacterBase) that indicates whether the 
	 * character is suffering from a burn debuff. This variable is set to true when a burn effect is applied 
	 * and false when it's removed. This function OnRep_Burned() is automatically called on clients when 
	 * bIsBurned changes due to replication.
	 * 
	 * These if/else branches handle both applying and removing burn effects:
	 * - true: Character just became burned → show burn visual effect
	 * - false: Burn effect ended → hide burn visual effect
	 */
	if (bIsBurned)
	{
		/**
		 * Activate the burn visual effect
		 * 
		 * BurnDebuffComponent is a UDebuffNiagaraComponent (defined in AFoxCharacterBase) that displays
		 * a Niagara particle system to visually indicate the character is burning
		 * 
		 * Activate() starts playing the particle system:
		 * - Spawns fire particles around the character
		 * - Provides visual feedback to players that the character is taking damage over time
		 * 
		 * The component remains active until Deactivate() is called when the burn ends
		 */
		BurnDebuffComponent->Activate();
	}
	// Check if the character does NOT have the burn debuff
	else
	{
		/**
		 * Deactivate the burn visual effect
		 * 
		 * BurnDebuffComponent is a UDebuffNiagaraComponent that was activated when the burn began
		 * 
		 * Deactivate() stops playing the particle system:
		 * - Removes the visual burn effect from the character
		 * - Provides clear visual feedback that the burn has ended
		 */
		BurnDebuffComponent->Deactivate();
	}
}

void AFoxCharacter::InitAbilityActorInfo()
{
	// Retrieve and cast the PlayerState to AFoxPlayerState to access Fox-specific player data and the Ability System Component
	AFoxPlayerState* FoxPlayerState = GetPlayerState<AFoxPlayerState>();
	
	// Validate that PlayerState exists - crashes in development builds if nullptr to catch critical setup errors early
	check(FoxPlayerState);
	
	/**
	 * Initialize the Ability Actor Info with Owner and Avatar actors
	 * 
	 * Breaking down this line:
	 * 1. FoxPlayerState->GetAbilitySystemComponent() - Retrieves the UAbilitySystemComponent pointer from PlayerState
	 * 2. ->InitAbilityActorInfo(...) - Calls the GAS framework function that sets up ability system actor references
	 * 
	 * Parameters explained:
	 * - InOwnerActor (FoxPlayerState): The actor that OWNS the Ability System Component
	 *   * For players, this is always the PlayerState because it persists across respawns
	 *   * The Owner is responsible for replication and ability state persistence
	 *   * PlayerState ownership ensures abilities/attributes survive character death
	 * 
	 * - InAvatarActor (this): The physical character that REPRESENTS the abilities visually
	 *   * 'this' refers to the AFoxCharacter instance calling this function
	 *   * The Avatar is the physical pawn/character in the world that executes abilities
	 *   * Visual effects, animations, and ability execution happen on the Avatar
	 *   * The Avatar can change (respawn, possess different character) while Owner stays the same
	 * 
	 * What InitAbilityActorInfo does internally:
	 * - Stores references to both Owner and Avatar actors
	 * - Initializes ability replication and networking based on Owner's network role
	 * - Prepares the ASC for ability activation, attribute changes, and gameplay effect application
	 * - MUST be called before using any GAS functionality (abilities, attributes, effects)
	 * 
	 * Multiplayer considerations:
	 * - Called on both Server (PossessedBy) and Client (OnRep_PlayerState)
	 * - Server: Sets up authoritative ability system with full replication rights
	 * - Client: Sets up local prediction and cosmetic ability execution
	 * - Owner actor determines replication authority (PlayerState is replicated actor)
	 */
	FoxPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(FoxPlayerState, this);
	
	/**
	 * Cast to UFoxAbilitySystemComponent and notify that AbilityActorInfo has been set
	 * 
	 * Breaking down this line:
	 * 1. FoxPlayerState->GetAbilitySystemComponent() - Returns a pointer to UAbilitySystemComponent (base class)
	 * 2. Cast<UFoxAbilitySystemComponent>(...) - Performs a safe cast to our custom UFoxAbilitySystemComponent subclass
	 *    - Cast<> is Unreal's template function for runtime type casting
	 *    - Returns nullptr if the cast fails (object is not of the target type)
	 *    - Returns a valid UFoxAbilitySystemComponent* pointer if successful
	 * 3. ->AbilityActorInfoSet() - Calls our custom function that broadcasts a delegate/event
	 * 
	 * Why this is necessary:
	 * - GetAbilitySystemComponent() returns the base UAbilitySystemComponent* type
	 * - AbilityActorInfoSet() is a custom function only available in UFoxAbilitySystemComponent
	 * - We must cast to the derived type to access our custom functionality
	 * 
	 * What AbilityActorInfoSet() does:
	 * - Broadcasts a delegate that notifies listeners that AbilityActorInfo has been initialized
	 * - UI widgets and other systems can bind to this delegate to know when GAS is ready
	 * - This allows UI elements to safely access ability data without nullptr crashes
	 * 
	 * Safety note:
	 * - No nullptr check here because we're certain this is a UFoxAbilitySystemComponent
	 * - The PlayerState is guaranteed to have the correct ASC type in our game architecture
	 * - If this cast fails, it indicates a critical setup error that should crash in development
	 */
	Cast<UFoxAbilitySystemComponent>(FoxPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	/**
	 * Cache the Ability System Component reference in the character
	 * 
	 * Why cache this reference:
	 * - Convenience: Allows calling AbilitySystemComponent directly instead of GetPlayerState()->GetAbilitySystemComponent()
	 * - Performance: Avoids repeated PlayerState lookups and function calls (minor optimization)
	 * - Base class requirement: AFoxCharacterBase expects AbilitySystemComponent to be assigned
	 * 
	 * Enemies create their ASC directly on the character (component-based approach)
	 * Players store ASC on PlayerState (persists through death/respawn)
	 * This cached reference unifies both approaches with a common interface
	 */
	AbilitySystemComponent = FoxPlayerState->GetAbilitySystemComponent();
	
	// Cache the Attribute Set reference in the character 
	AttributeSet = FoxPlayerState->GetAttributeSet();
	
	/**
	 * Broadcast the OnAscRegistered delegate to notify listeners that the ASC is ready
	 * 
	 * This delegate is crucial for components that need to interact with the Ability System Component
	 * but might be created before the ASC is initialized (like UDebuffNiagaraComponent).
	 * 
	 * Example workflow with UDebuffNiagaraComponent:
	 * 1. DebuffNiagaraComponent is created and BeginPlay() is called
	 * 2. At that time, the character might not have its ASC initialized yet
	 * 3. DebuffNiagaraComponent checks if ASC exists. If not, it binds to OnAscRegistered delegate
	 * 4. When this broadcast fires, the lambda in DebuffNiagaraComponent executes
	 * 5. The component can now safely register for gameplay tag events (e.g., debuff application/removal)
	 * 6. This enables the visual effect to activate/deactivate based on debuff tag count changes
	 * 
	 * Without this delegate:
	 * - Components would need polling or complex timing logic to know when ASC is ready
	 * - Race conditions could occur where components try to access an uninitialized ASC
	 * - Gameplay tag event registration would fail, breaking debuff visual feedback
	 * 
	 * The delegate is defined in ICombatInterface and implemented in AFoxCharacterBase
	 */
	OnAscRegistered.Broadcast(AbilitySystemComponent);
	
	/**
	 * Register a callback for when the Debuff_Stun gameplay tag is added or removed from the ASC
	 * 
	 * Breaking down this line:
	 * 1. AbilitySystemComponent->RegisterGameplayTagEvent(...) - Registers a delegate that fires when a specific tag changes
	 * 2. FFoxGameplayTags::Get().Debuff_Stun - The gameplay tag we're monitoring (represents stun debuff state)
	 * 3. EGameplayTagEventType::NewOrRemoved - Event type that triggers when:
	 *    - NewOrRemoved: Fires when tag count goes from 0 to 1 (added) OR from 1 to 0 (removed)
	 *    - Alternative types: NewOrIncremented (fires on every add), RemovedOrDecremented (fires on every remove)
	 * 4. .AddUObject(this, &AFoxCharacter::StunTagChanged) - Binds the StunTagChanged function as the callback
	 *    - 'this': The object instance that owns the callback function (this AFoxCharacter)
	 *    - &AFoxCharacter::StunTagChanged: Pointer to the member function that will be called when the tag count changes
	 *    and the delegate broadcasts
	 * 
	 * What this accomplishes:
	 * - Monitors the ASC for changes to the Debuff_Stun tag count
	 * - When a stun effect is applied (tag added), StunTagChanged fires with NewCount > 0 (NewCount is an input parameter
	 *   of the callback function that the delegate will pass this function. It is the the new tag count)
	 * - When all stun effects expire (tag removed), StunTagChanged fires with NewCount = 0
	 * - StunTagChanged then applies/removes input blocking tags and stun visual effects
	 * 
	 * Note: The callback function StunTagChanged receives:
	 * - CallbackTag: The tag that changed (Debuff_Stun in this case)
	 * - NewCount: The new reference count for this tag (0 = removed, >0 = active)
	 */
	AbilitySystemComponent->RegisterGameplayTagEvent(FFoxGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AFoxCharacter::StunTagChanged);
	
	/**
	 * Initialize the player's HUD overlay after Ability System is ready
	 * 
	 * GetController() returns the AController that is possessing this character
	 * Cast<AFoxPlayerController>(...) safely casts it to our custom player controller class
	 * 
	 * Why this cast might fail (return nullptr):
	 * - This function is called on both server and clients (PossessedBy on server, OnRep_PlayerState on clients)
	 * - AI-controlled characters don't have an AFoxPlayerController (they use AIController instead)
	 * - During level transitions or respawns, the controller might not be fully initialized yet
	 * 
	 * This check ensures we only proceed with UI initialization for player-controlled characters
	 */
	if (AFoxPlayerController* FoxPlayerController = Cast<AFoxPlayerController>(GetController()))
	{
		/**
		 * Get the HUD and initialize the overlay widget
		 * 
		 * FoxPlayerController->GetHUD() retrieves the AHUD associated with this player controller
		 * Cast<AFoxHUD>(...) safely casts it to our custom HUD class that manages UI widgets
		 * 
		 * InitOverlay() is called to set up the player's UI overlay with GAS data bindings:
		 * - FoxPlayerController: Allows the UI to respond to player input and controller events
		 * - FoxPlayerState: Provides access to persistent player data (level, XP, etc.)
		 * - AbilitySystemComponent: Enables UI to listen for ability activations, cooldowns, and costs
		 * - AttributeSet: Allows UI to bind to and display attribute values (health, mana, etc.)
		 * 
		 * Why initialize UI here:
		 * - AbilityActorInfo must be fully initialized before UI can safely access GAS data
		 * - This is the earliest point where all required references are guaranteed to be valid
		 * - UI widgets can now bind delegates to attribute changes and display current values
		 * 
		 * This only executes on the owning client (the player's local machine) because:
		 * - GetHUD() returns nullptr on the server and other clients
		 * - Only the local player controller has a valid HUD reference
		 */
		if (AFoxHUD* FoxHUD = Cast<AFoxHUD>(FoxPlayerController->GetHUD()))
		{
			FoxHUD->InitOverlay(FoxPlayerController, FoxPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	/**
	 * Apply default attribute values using Gameplay Effects
	 * 
	 * InitializeDefaultAttributes() is defined in AFoxCharacterBase and applies Gameplay Effects
	 * that set the character's starting attribute values (Health, Mana, Strength, etc.)
	 * 
	 * This must be called AFTER AbilityActorInfo is initialized because:
	 * - Gameplay Effects require a valid AbilitySystemComponent to be applied
	 * - The ASC needs to know its Owner and Avatar before processing attribute modifications
	 * 
	 * What happens inside:
	 * - Applies Primary Attributes (Strength, Intelligence, etc.) based on character class
	 * - Applies Secondary Attributes (Armor, Critical Hit Chance, etc.) derived from primaries
	 * - Applies Vital Attributes (Health, Mana) and set them to their maximum values
	 * 
	 * This initialization is permanent for the character's lifetime and forms the base values
	 * from which all subsequent attribute calculations are derived.
	 */
	InitializeDefaultAttributes();
}


