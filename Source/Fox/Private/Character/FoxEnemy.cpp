// Copyright TryingToMakeGames


#include "Character/FoxEnemy.h"

#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "AbilitySystem/FoxAttributeSet.h"
#include "AI/FoxAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Fox/Fox.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/FoxUserWidget.h"

AFoxEnemy::AFoxEnemy()
{
	/*
	 * Configure the enemy's skeletal mesh to block the Visibility collision channel, which is essential for cursor-based
	 * interaction and targeting systems.
	 * 
	 * What is ECC_Visibility?
	 * - ECC_Visibility is a trace channel specifically designed for line traces that determine what objects are "visible"
	 *   or interactable from the player's perspective
	 * - It's commonly used for cursor traces (like clicking on objects), camera visibility checks, and targeting systems
	 * - By default, many objects ignore this channel, so we must explicitly configure responses for interactive objects
	 * 
	 * What does ECR_Block mean?
	 * - ECR_Block (Collision Response: Block) means this mesh will stop/block traces on the Visibility channel
	 * - When a line trace on the Visibility channel hits this mesh, it will register a hit and stop the trace
	 * - This is different from ECR_Overlap (trace passes through but registers hit) or ECR_Ignore (trace passes through
	 *   completely)
	 * 
	 * Why is this important for enemies?
	 * - This allows the player's cursor to "hit" and select the enemy when hovering over it in the game world
	 * - Essential for click-to-target systems, allowing players to select enemies for attack or interaction
	 * - Works in conjunction with the HighlightActor() system - when the cursor trace hits this mesh, we can trigger
	 *   the custom depth highlighting to visually indicate the enemy is targetable
	 * - Without this setting, cursor traces would pass through the enemy and fail to detect it as an interactive target
	 * 
	 * How it works with our highlight system:
	 * 1. Player moves cursor over enemy in viewport
	 * 2. Game performs a line trace on the ECC_Visibility channel from cursor into world
	 * 3. Trace hits this enemy's mesh because we set collision response to Block
	 * 4. Hit result triggers HighlightActor() which enables custom depth rendering
	 * 5. Post-process material draws red outline around enemy mesh and weapon
	 * 6. When cursor moves away, UnHighlightActor() is called to remove the effect
	 */
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	// We use the base UAbilitySystemComponent type in the parent class (FoxCharacterBase) because it allows
	// polymorphism - different child classes can use different AbilitySystemComponent implementations. 
	// For example, AFoxEnemy uses UFoxAbilitySystemComponent, but another child class could use a different 
	// ASC type. The parent class only needs to know about the base interface (UAbilitySystemComponent), 
	// not the specific implementation, which follows the Dependency Inversion Principle and keeps the base 
	// class flexible and reusable. The string "AttributeSet" is the unique object name for this component.
	AbilitySystemComponent = CreateDefaultSubobject<UFoxAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	
	// Disable direct controller rotation on all axes - the enemy won't instantly snap to face the controller's rotation.
	// This is important for AI-controlled characters because we want smooth, natural-looking movement rather than
	// instant rotation changes that would look robotic.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	// Get the character movement component and
	// Enable controller desired rotation, which makes the character smoothly rotate towards the direction the AI 
	// controller wants to move. This works in conjunction with the rotation rate settings in CharacterMovement to 
	// create natural-looking turns as the enemy navigates and chases the player. The AI sets a desired rotation 
	// and the character smoothly interpolates towards it each frame.
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	
	// Create and initialize the AttributeSet component which contains all the gameplay attributes (Health, MaxHealth,
	// Mana, Strength, etc.) for this enemy. The AttributeSet works with the AbilitySystemComponent to store attribute
	// values, respond to attribute changes through GameplayEffects, and provide attribute accessor functions. This must
	// be created before InitAbilityActorInfo() is called so the ASC can properly register and track these attributes.
	// The string "AttributeSet" is the unique object name for this component.
	AttributeSet = CreateDefaultSubobject<UFoxAttributeSet>("AttributeSet");

	// Create the HealthBar widget component which will display a 3D health bar above the enemy's head in the game world.
	// UWidgetComponent is a special component that renders a UMG widget in 3D space, allowing UI elements to exist as
	// part of the game world rather than screen-space overlay. This health bar will update automatically when the enemy's
	// health changes through delegate bindings established in BeginPlay().
	// The string "HealthBar" is the unique object name for this component.
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");

	// Attach the HealthBar widget component to the root component (the CapsuleComponent inherited from ACharacter) so it
	// moves, rotates, and scales with the enemy actor. SetupAttachment() establishes a parent-child transform hierarchy,
	// meaning the HealthBar's world transform will be relative to the root component's transform, ensuring the health bar
	// always stays positioned correctly above the enemy even as the enemy moves around the level.
	HealthBar->SetupAttachment(GetRootComponent());
}

void AFoxEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// If we are not on the server, return early. AI is only controlled by the server. Anything clients see is a result 
	// of replication. So we do not want to perform the following AI related operations on clients
	if (!HasAuthority()) return;
	
	// Cast the NewController passed into this function to our custom AFoxAIController type and store it in the 
	// FoxAIController variable.
	FoxAIController = Cast<AFoxAIController>(NewController);
	
	/*
	   Initialize the AI Controller's Blackboard Component with the Blackboard Asset referenced by our BehaviorTree.
	   The Blackboard is a key/value storage system that the Behavior Tree uses to store and retrieve data during
	   AI decision making (like target location, current state, patrol points, etc.). Each BehaviorTree has an
	   associated BlackboardAsset that defines what keys/variables are available. 

	   We dereference (*) the BehaviorTree->BlackboardAsset pointer to pass the actual UBlackboardData object to 
	   InitializeBlackboard(). The BlackboardAsset is a data asset that you create in the Unreal Editor (usually 
	   named something like BB_EnemyName). Inside this asset, you define all the keys (variables) that the AI will 
	   use - for example, you might define a key called "TargetActor" of type Object, "PatrolLocation" of type Vector, 
	   or "IsChasing" of type Bool. 

	   When InitializeBlackboard() is called, it reads through all the key definitions in the BlackboardAsset and 
	   creates actual storage space in the Blackboard Component for each one. It allocates memory for each key based 
	   on its type and sets them to their default values. After this initialization, the Blackboard Component now has 
	   all these keys available as empty slots ready to store data. The Behavior Tree's tasks can then write values 
	   to these keys (like setting "TargetActor" to the player character) and read from them (like checking if 
	   "IsChasing" is true) during AI execution.

	   This must be done before running the behavior tree, as the tree's tasks and decorators will need to read/write 
	   blackboard values.
	*/
	FoxAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);

	/*
	   Start executing the Behavior Tree by passing our BehaviorTree asset to the AI Controller's RunBehaviorTree
	   function. This begins the AI's decision-making process where the behavior tree will continuously evaluate
	   its nodes (composites, decorators, tasks, services) to determine what actions the enemy should take.
	   The behavior tree will run every frame, checking conditions and executing behaviors based on the tree's
	   structure and the current state of the blackboard. This is the final step in setting up the enemy's AI
	   after the blackboard has been initialized with the proper data structure.
	 */
	FoxAIController->RunBehaviorTree(BehaviorTree);
	
	// Get the blackboard component associated with this AI Controller and set the value of the HitReacting blackboard key
	// to false
	FoxAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	
	// Set the value of the RangedAttacker blackboard key based on the character class, if it is anything other than
	// warrior then the value is set to true (since Ranger and Elementalis are ranged attackers)
	FoxAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

void AFoxEnemy::HighlightActor()
{
	/*
	 * Custom Depth Rendering and Stencil System for Actor Highlighting
	 * 
	 * What is Custom Depth?
	 * - Custom Depth is a special rendering pass in Unreal Engine that renders objects to a separate buffer
	 * - This buffer (called the Custom Depth buffer) can be accessed in post-process materials to create effects
	 * - When enabled on a mesh, that mesh is rendered into this separate depth buffer independently of the main scene
	 * - Think of it like a "mask" or "selection layer" in Photoshop - it marks which pixels belong to this object
	 * 
	 * What is the Stencil Value?
	 * - The Custom Depth Stencil is an 8-bit integer value (0-255) you can assign to each mesh
	 * - This value is stored per-pixel in the Custom Depth buffer alongside the depth information
	 * - Different objects can have different stencil values to identify or color them differently
	 * - In our case: CUSTOM_DEPTH_RED is a macro (defined in Fox.h) representing a specific integer value
	 * - This value tells our post-process material to render this object with a red highlight color
	 * 
	 * How Does the Highlighting Work?
	 * 1. We enable Custom Depth rendering on the mesh (tells engine to render it to Custom Depth buffer)
	 * 2. We assign a stencil value (CUSTOM_DEPTH_RED) that identifies this as a "red highlight" object
	 * 3. A Post-Process Material (set up in the project) reads the Custom Depth buffer every frame
	 * 4. The post-process material checks each pixel's stencil value
	 * 5. If it finds our CUSTOM_DEPTH_RED value, it draws a red outline/glow effect around that object
	 * 6. This happens after all normal rendering, as a screen-space effect overlaid on the final image
	 * 
	 * Why Both Mesh and Weapon?
	 * - The character's body (GetMesh) and weapon are separate mesh components
	 * - We want the entire enemy (body + weapon) to highlight when targeted
	 * - So we enable custom depth and set the same stencil value on both components
	 * - This ensures a unified highlight effect across the complete character
	 * 
	 * Common Use Cases:
	 * - Highlighting interactable objects when the player looks at them
	 * - Showing enemy outlines through walls (wallhack effect)
	 * - Team-colored outlines in multiplayer games
	 * - Selection indicators in strategy games
	 */

	// Enable custom depth rendering for the character's skeletal mesh, allowing it to be rendered to the Custom Depth buffer
	GetMesh()->SetRenderCustomDepth( true);
	
	// Set the custom depth stencil value to CUSTOM_DEPTH_RED, which tells the post-process material to render this mesh with a red highlight
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	
	// Enable custom depth rendering for the weapon mesh, allowing it to be rendered to the Custom Depth buffer
	Weapon->SetRenderCustomDepth(true);
	
	// Set the custom depth stencil value to CUSTOM_DEPTH_RED for the weapon, ensuring it matches the character's highlight color
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AFoxEnemy::UnHighlightActor()
{
	// Disable custom depth rendering for the character's skeletal mesh, which removes the highlight effect
	GetMesh()->SetRenderCustomDepth( false);
	
	// Disable custom depth rendering for the weapon mesh, which removes the highlight effect from the weapon
	Weapon->SetRenderCustomDepth(false);
}

int32 AFoxEnemy::GetPlayerLevel_Implementation()
{
	// Returns the Level of this enemy
	return Level;
}

void AFoxEnemy::Die(const FVector& DeathImpulse)
{
	// Set the lifespan of this actor. When it expires the object will be destroyed.
	SetLifeSpan(LifeSpan);
	
	// Checks if the ai controller is valid and sets the Dead blackboard key value to true
	if (FoxAIController) FoxAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	
	// Calls the parent class' (FoxCharacterBase) Die function and passes in a DeathImpulse parameter which is a vector representing
	// the force and direction to apply to the character's ragdoll physics upon death.
	Super::Die(DeathImpulse);
}

void AFoxEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	// Sets the combat target for this enemy
	CombatTarget = InCombatTarget;
}

AActor* AFoxEnemy::GetCombatTarget_Implementation() const
{
	// Returns the current combat target for this enemy
	return CombatTarget;
}

void AFoxEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// Sets the max walk speed of this class to the value of the BaseWalkSpeed member variable 
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	
	InitAbilityActorInfo();
	
	// Checks if we are on the server
	if (HasAuthority())
	{
		// Call the function to give the enemy startup abilities
		UFoxAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
		
	// Initialize the health bar widget by setting this enemy actor as its widget controller.
	// This is a critical setup step that establishes the connection between the UI and the data source:
	// 1. GetUserWidgetObject() retrieves the actual UMG widget instance from the HealthBar UWidgetComponent
	// 2. Cast<UFoxUserWidget>() attempts to safely cast it to our custom UFoxUserWidget type, which has 
	//    the ability to accept and work with widget controllers
	// 3. If the cast succeeds (the widget is indeed a UFoxUserWidget), we call SetWidgetController(this)
	//    passing the enemy actor itself as the controller
	// 4. This allows the health bar widget to access this enemy's delegates (OnHealthChanged, OnMaxHealthChanged)
	//    and bind to them so it can automatically update the visual health bar whenever health values change
	// Without this setup, the widget would exist but have no way to receive or display health data updates
	if (UFoxUserWidget* FoxUserWidget = Cast<UFoxUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		FoxUserWidget->SetWidgetController(this);
	}
	
	// Cast the AttributeSet to FoxAttributeSet and check if it is valid
	if (const UFoxAttributeSet* FoxAS = Cast<UFoxAttributeSet>(AttributeSet))
	{
		// Bind a Lambda function (just a function defined in place right here) to delegate for the Health attribute
		// broadcast by ASC when this attribute changes. Then the Lambda callback that will execute when this delegate 
		// broadcasts will broadcast its own delegate which we will bind to in blueprint so the widget can receive the 
		// new health value
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(FoxAS->GetHealthAttribute()).AddLambda(
				[this](const FOnAttributeChangeData& Data)
				{
					OnHealthChanged.Broadcast(Data.NewValue);
				}
		);	
		
		// See comment above but this is for MaxHealth
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(FoxAS->GetMaxHealthAttribute()).AddLambda(
				[this](const FOnAttributeChangeData& Data)
				{
					OnMaxHealthChanged.Broadcast(Data.NewValue);
				}
		);	
		
		/*
		 * Bind callback to delegate on the ASC that broadcasts whenever a gameplay tag is added to the ASC
		 * If you go to the declaration of RegisterGameplayTagEvent and then go to the declaration of its return type
		 * FOnGameplayEffectTagCountChanged we can see that callbacks we wish to bind to this delegate must accept
		 * the input parameters const FGameplayTag and int32 as shown below. The int32 is the count of this specific tag
		 * since the ASC can have multiple instances of the same tag. When the delegate broadcasts the callback receives
		 * both the FGameplayTag (the tag that changed) and the int32 (the new count of that tag)
		 * DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameplayEffectTagCountChanged, const FGameplayTag, int32);
		 * 
		 * 'FFoxGameplayTags::Get().Effects_HitReact' gets the singleton instance of FFoxGameplayTags which contains all
		 * the native gameplay tags and then gets the Effects_HitReact tag from it. Passing this as the first parameter
		 * makes our callback function only be called when the delegate broadcasts this tag. The second parameter 
		 * 'EGameplayTagEventType::NewOrRemoved' makes our callback function only be called when the tag is added or 
		 * completely removed (not just a single instance of that tag) from the ASC.
		 * 
		 * We then bind the callback function to the delegate using the input parameters:
		 * InUserObject — User object to bind to ('this' is the AFoxEnemy object that the callback function is a member of)
		 * InFunc — Class method function address (the address of the HitReactTagChanged function)
		*/
		AbilitySystemComponent->RegisterGameplayTagEvent(FFoxGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&AFoxEnemy::HitReactTagChanged
		);
		
		// Broadcast the initial values for Health and MaxHealth
		OnHealthChanged.Broadcast(FoxAS->GetHealth());
		OnMaxHealthChanged.Broadcast(FoxAS->GetMaxHealth());
	}
}

void AFoxEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// Sets this member variable to true if the HitReact tag's count is greater than 0
	bHitReacting = NewCount > 0;
	
	// If the enemy is currently hit reacting, set the max walk speed to 0, otherwise set it to the base walk speed
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	
	// Perform a null-check safety validation before attempting to access the blackboard component. This two-part check
	// ensures that: 1) FoxAIController pointer is valid (the enemy has been possessed by an AI controller), and
	// 2) The AI controller's blackboard component exists and has been properly initialized. Without this validation,
	// attempting to call GetBlackboardComponent() on a null FoxAIController or calling SetValueAsBool on a null
	// blackboard component would cause a crash. This situation could occur during initialization before PossessedBy
	// is called, or if the AI controller setup fails for any reason.
	if (FoxAIController && FoxAIController->GetBlackboardComponent())
	{
		// Update the HitReacting blackboard key with the current bHitReacting state. This communicates to the
		// Behavior Tree that the enemy is currently playing a hit reaction animation, which allows the BT to make
		// decisions based on this state (e.g., pausing movement, canceling attacks, or waiting until the reaction
		// finishes before resuming normal AI behavior). The blackboard acts as shared memory between C++ code and
		// the visual Behavior Tree logic in the editor.
		FoxAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AFoxEnemy::InitAbilityActorInfo()
{
	// Initialize the Ability System Component by setting both the Owner Actor and Avatar Actor to 'this' enemy.
	// The first 'this' parameter (Owner Actor) is the actor that OWNS the ASC - in this case the enemy itself.
	// The second 'this' parameter (Avatar Actor) is the physical representation in the world - also the enemy itself.
	// For enemies, both are the same because the ASC lives directly on the enemy actor. This differs from the player
	// character where the Owner is the PlayerState (for replication) but the Avatar is the Character (for location/mesh).
	// This initialization must happen before any abilities or gameplay effects can be used on this enemy.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// Cast the base UAbilitySystemComponent to our custom UFoxAbilitySystemComponent type so we can call 
	// AbilityActorInfoSet(), which is a custom function defined in our derived class. The cast is necessary because
	// the AbilitySystemComponent member variable is declared as UAbilitySystemComponent* in the parent class for
	// polymorphism purposes, but we need access to UFoxAbilitySystemComponent-specific functionality. AbilityActorInfoSet()
	// broadcasts a delegate that allows other systems (like UI or ability-specific logic) to respond when the ASC's
	// actor info has been fully initialized and is ready to use. This notification pattern ensures dependent systems
	// initialize in the correct order.
	Cast<UFoxAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	
	// Checks if we are on the server
	if (HasAuthority())
	{
		// Calls the function to initialize default attributes
		InitializeDefaultAttributes();
	}
	
	// Broadcast the OnAscRegistered delegate to notify any listeners (such as DebuffNiagaraComponents) that this
	// enemy's Ability System Component has been fully initialized and registered. This allows components that need
	// to interact with the ASC (like setting up gameplay tag event listeners) to do so at the correct time, even if
	// they were created before the ASC was ready. This is particularly important for debuff visual effects that need
	// to register for tag change notifications but might be added to the actor before InitAbilityActorInfo() is called.
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void AFoxEnemy::InitializeDefaultAttributes() const
{
	// This function gets the data asset stored on the game mode, gets the class info based on the character class and 
	// applies gameplay effects based on the enemies class and level to initialize the default attributes
	UFoxAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
