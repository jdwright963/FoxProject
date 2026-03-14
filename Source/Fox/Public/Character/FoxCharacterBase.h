// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

// Is this needed?
//#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "FoxCharacterBase.generated.h"

class UDebuffNiagaraComponent;
class UNiagaraSystem;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;
class UAnimMontage;


// IAbilitySystemInterface is an Unreal Engine interface that provides a standardized way to access 
// an actor's Ability System Component (ASC). By implementing this interface, our character can work 
// with the Gameplay Ability System (GAS) and allows other systems to query for the ASC using 
// GetAbilitySystemComponent(). This is essential for GAS to function properly, as many GAS functions 
// expect actors to implement this interface to retrieve their ASC for applying gameplay effects, 
// granting abilities, and managing attributes.

// WHY GAMEPLAYABILITIES IS IN PUBLIC DEPENDENCIES (Fox.Build.cs):
// 
// We need "GameplayAbilities" in our PublicDependencyModuleNames because this header file (FoxCharacterBase.h) 
// is PUBLIC and exposes GAS types in its interface:
// - We inherit from IAbilitySystemInterface (from GameplayAbilities module)
// - We have forward declarations for UAbilitySystemComponent and UAttributeSet (both from GameplayAbilities)
// - We return these types in public methods like GetAbilitySystemComponent() and GetAttributeSet()
//
// PUBLIC vs PRIVATE DEPENDENCIES:
// - PUBLIC dependencies: Required when a module's PUBLIC headers (.h files) expose types from another module.
//   Any module that includes our public headers will transitively get access to our public dependencies.
//   Use when: Your header files use types, inherit from classes, or have members from the dependency.
//
// - PRIVATE dependencies: Required when only IMPLEMENTATION files (.cpp) use types from another module,
//   but those types are NOT exposed in any public headers.
//   Use when: You only use the dependency's types internally in .cpp files.
//
// Since FoxCharacterBase.h exposes IAbilitySystemInterface, UAbilitySystemComponent, and UAttributeSet 
// in its public interface, any code that includes this header needs access to GameplayAbilities types.
// Therefore, GameplayAbilities MUST be a public dependency.
//
// Compare this to GameplayTags and GameplayTasks in our PrivateDependencyModuleNames - we only use those 
// in .cpp implementation files, never exposing their types in our public headers.
UCLASS(Abstract)
class FOX_API AFoxCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFoxCharacterBase();
	
	// Overridden from AActor to specify which properties should be replicated across the network.
	// This function is called by the engine's replication system to register which variables should be replicated
	// between the server and clients. In this class, we register bIsStunned and bIsBurned for replication, ensuring
	// that when these debuff states change on the server, clients are automatically notified and can respond with
	// appropriate visual/audio feedback through their OnRep callbacks (OnRep_Stunned and OnRep_Burned).
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	
	// This function must be overridden to implement IAbilitySystemInterface. The getter for the attribute set
	// was made for convenience.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// Function that returns the character's attribute set
	UAttributeSet* GetAttributeSet() const { return AttributeSet; };
	
	/** Combat Interface */
	
	// This function must be overridden to implement ICombatInterface. It returns the hit reaction animation montage
	// we wish to play when the character is hit. Child classes store the animation montage they wish to play
	// in that variable
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;	
	
	// Function for character death from the CombatInterface and overridden here
	// This handles what happens on the server when the character dies. Takes a DeathImpulse parameter which is a vector
	// representing the force and direction to apply to the character's ragdoll physics upon death.
	virtual void Die(const FVector& DeathImpulse) override;	
	
	// This is a BlueprintNativeEvent function from CombatInterface that is overridden here. It returns the location of
	// the character's combat socket associated with a certain tag
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
	
	// These are BlueprintNativeEvent functions from CombatInterface that are overridden here. They check if the actor
	// who implements the interface is dead or not, and return the actor that implements the interface
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	
	// Overrided function from CombatInterface that returns an array of attack animation montages with their associated 
	// gameplay tags
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
	
	// Overrided function from CombatInterface that returns the Niagara system for blood effects
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;
	
	// Overrided function from CombatInterface that returns an instance of FTaggedMontage with a specific MontageTag
	// from the AttackMontages array
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
	
	// Overrided function from CombatInterface that returns the number of minions the character has spawned
	virtual int32 GetMinionCount_Implementation() override;
	
	// Overrided function from CombatInterface that increments the minion count by a specified amount
	virtual void IncrementMinionCount_Implementation(int32 Amount) override;
	
	// Overrided function from CombatInterface that returns the character's RPG class (Warrior, Elementalist, Ranger, etc.)
	virtual ECharacterClass GetCharacterClass_Implementation() override;
	
	// Overrided function from CombatInterface that returns a reference to the OnAscRegistered delegate, allowing external
	// systems (like UDebuffNiagaraComponent) to bind callbacks that execute when this character's Ability System Component
	// is registered and initialized. This is particularly useful for components that need the ASC but are created before
	// the ASC is available (e.g., on enemies where ASC initialization happens in BeginPlay).
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override;

	// Overrided function from CombatInterface that returns a reference to the FOnDeathSignature delegate, allowing external systems
	// (like UDebuffNiagaraComponent) to bind callbacks that execute when this character dies. This enables other components
	// and systems to respond to death events (e.g., deactivating particle effects, playing sounds, updating UI).
	virtual FOnDeathSignature& GetOnDeathDelegate() override;
	
	// Overrided function from CombatInterface that returns the character's weapon skeletal mesh component. This provides
	// external systems access to the weapon mesh for various purposes such as retrieving socket locations for projectile
	// spawning (e.g., WeaponTipSocketName), attaching visual effects (e.g., weapon trails, elemental effects), performing
	// weapon swapping or customization, and animating weapon-specific behaviors.
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;
	
	// Overrided function from CombatInterface that sets the value of the variable that indicates whether this character
	// is currently being shocked by the Electrocute ability. When the input parameter bInShock is true, the character
	// enters the shocked state. When false, the shocked state is cleared. This function updates the bIsBeingShocked 
	// replicated variable, which automatically notifies all clients of the state change through the replication system, 
	// allowing them to respond with appropriate visual and gameplay effects (e.g., activating shock particle effects, 
	// applying movement restrictions).
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;

	// Overrided function from CombatInterface that returns whether this character is currently being shocked.
	// This function provides read-only access to the bIsBeingShocked state, allowing external systems (like AI
	// behavior trees, UI elements, or other gameplay systems) to query if the character is affected by the Electrocute
	// ability without modifying the state.
	virtual bool IsBeingShocked_Implementation() const override;
	
	/** end Combat Interface */
	
	// Delegate that broadcasts when this character's Ability System Component has been registered and initialized.
	// External systems can bind to this delegate via GetOnASCRegisteredDelegate() to receive a callback with the
	// initialized ASC, which is useful when components need the ASC but are created before it's available.
	// For example, UDebuffNiagaraComponent uses this to register gameplay tag listeners once the ASC is ready.
	// This delegate type is inherited from CombatInterface
	FOnASCRegistered OnAscRegistered;

	// Delegate that broadcasts when this character dies. External systems can bind to this delegate via
	// GetOnDeathDelegate() to receive a callback when death occurs, allowing them to respond appropriately
	// (e.g., UDebuffNiagaraComponent deactivates particle effects, UI systems update death counts, etc.).
	// The delegate passes the dead actor as a parameter to provide context to bound callbacks.
	// This delegate type is inherited from CombatInterface
	FOnDeathSignature OnDeathDelegate;
	
	// Multicast RPC (called on the server and executed on the server and every client currently connected to the game)
	// that handles what happens on all clients when the character dies. Takes a DeathImpulse parameter which is a vector representing
	// the force and direction to apply to the character's ragdoll physics upon death.
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);
	
	// Array storing all attack animation montages for this character, where each montage is tagged with a gameplay tag
	// for identification. Uses the FTaggedMontage struct (defined in CombatInterface.h) which pairs an animation montage
	// with a gameplay tag. The value for this array is set in the blueprint, where designers can add
	// multiple attack animations and assign appropriate tags to each one for the combat system to reference
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;
	
	// Replicated boolean that tracks whether this character is currently stunned. When the stun debuff is applied,
	// this variable is set to true on the server and automatically replicated to all clients, calling the
	// OnRep_Stunned() callback on each client. When the stun debuff is removed, it's set to false and replicated
	// again. This allows clients to respond to stun state changes (e.g., activating particle effects, disabling movement, 
	// playing animations, updating UI). The ReplicatedUsing specifier ensures OnRep_Stunned() is called on clients whenever this value changes.
	UPROPERTY(ReplicatedUsing=OnRep_Stunned, BlueprintReadOnly)
	bool bIsStunned = false;

	// Replicated boolean that tracks whether this character is currently burning. When the burn debuff is applied,
	// this variable is set to true on the server and automatically replicated to all clients, calling the
	// OnRep_Burned() callback on each client. When the burn debuff is removed, it's set to false and replicated
	// again. This allows clients to respond to burn state changes (e.g., activating particle effects via
	// BurnDebuffComponent, playing sound effects, updating UI). The ReplicatedUsing specifier ensures OnRep_Burned()
	// is called on clients whenever this value changes.
	UPROPERTY(ReplicatedUsing=OnRep_Burned, BlueprintReadOnly)
	bool bIsBurned = false;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBeingShocked = false;

	// Replication notification callback that executes on clients when the bIsStunned variable changes on the server.
	// This function is automatically called by the replication system after bIsStunned has been updated on the client,
	// allowing the client to respond to stun state changes. Typically used to update visual feedback (animations,
	// particle effects) or gameplay state (movement restrictions) when a character becomes stunned or recovers from stun.
	// Virtual to allow child classes (like AFoxEnemy) to override and add specific stun response behavior. There is no
	// implementation for this function in this class, but child classes can override it to provide custom stun handling.
	UFUNCTION()
	virtual void OnRep_Stunned();

	// Replication notification callback that executes on clients when the bIsBurned variable changes on the server.
	// This function is automatically called by the replication system after bIsBurned has been updated on the client,
	// allowing the client to respond to burn state changes. Typically used to activate/deactivate burn particle effects
	// (via BurnDebuffComponent), play burn-related sounds, or update UI elements when a character starts or stops burning.
	// Virtual to allow child classes to override and add specific burn response behavior. There is no implementation for this
	// function in this class, but child classes can override it to provide custom stun handling.
	UFUNCTION()
	virtual void OnRep_Burned();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// The character's weapon component. The value for this is set in the blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	// The name of the socket on the tip of the character's weapon. For example, this is where spells will be spawned
	// The value for this is set in the blueprint. The actual socket on the characters weapon must also be created 
	// in the editor and have the exact as the value set in the blueprint for this property.
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;
	
	// Socket on a characters left hand. This is for characters that attack with their hands and not a weapon
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName LeftHandSocketName;

	// Socket on a characters right hand. This is for characters that attack with their hands and not a weapon
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName RightHandSocketName;
	
	// Socket on a characters tail. This is for characters that attack with their tail and not a weapon
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName TailSocketName;
	
	// Variable to track if the character is dead
	bool bDead = false;
	
	// Callback function that is called by the engine-defined ASC delegate that broadcasts when GameplayTags on the
	// ASC change. This function is bound to be called when the Debuff.Stun tag is added or removed (when its stack
	// count changes from or to zero) from the ASC. It handles stun state changes by updating the bIsStunned replicated
	// variable and modifying the character's movement speed. When stunned (NewCount > 0), movement speed is set to 0
	// and bIsStunned is set to true. When unstunned (NewCount == 0), movement speed is restored to BaseWalkSpeed and
	// bIsStunned is set to false. Virtual to allow child classes (like AFoxEnemy) to override and add additional
	// stun response behavior.
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// The character's base walking speed in units per second. This value is used as the default movement speed when
	// the character is not affected by any movement-altering effects (like stun, slow, etc.). When movement speed
	// modifying effects are removed, the character's speed is restored to this base value. The value can be set in
	// the blueprint to allow designers to configure different movement speeds for different character types.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 600.f;
	
	// We use the base UAbilitySystemComponent type in the parent class (FoxCharacterBase) because it allows
	// polymorphism - different child classes can use different AbilitySystemComponent implementations. 
	// For example, AFoxEnemy uses UFoxAbilitySystemComponent, but another child class could use a different 
	// ASC type. The parent class only needs to know about the base interface (UAbilitySystemComponent), 
	// not the specific implementation, which follows the Dependency Inversion Principle and keeps the base 
	// class flexible and reusable.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// The character's attribute set, which stores gameplay attributes like Health, Mana, Strength, etc.
	// The AttributeSet works in tandem with the AbilitySystemComponent - while the ASC manages abilities and effects,
	// the AttributeSet holds the actual attribute values that those abilities and effects modify. Together they form
	// the core of the Gameplay Ability System for this character.
	//
	// Similar to AbilitySystemComponent, we use the base UAttributeSet type in the parent class to allow polymorphism.
	// Different child classes can use different AttributeSet implementations (e.g., UFoxAttributeSet with specific
	// attributes for player characters, or a different set for enemies). The parent class only needs to know about
	// the base interface, following the Dependency Inversion Principle and keeping the base class flexible.
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	// Initializes the Ability System Component's actor information by establishing the relationship between
	// the ASC, its owner actor, and its avatar actor. This is a critical setup function that must be called
	// before the ability system can function properly.
	//
	// This function is virtual because different character types need different initialization logic:
	// - Player characters (AFoxCharacter) retrieve their ASC from the PlayerState and initialize it there
	// - AI characters (AFoxEnemy) create and initialize their ASC directly on themselves
	// - Other character types might have entirely different initialization requirements
	//
	// The function is typically called during character possession (for players via PossessedBy/OnRep_PlayerState)
	// or during BeginPlay (for AI characters).
	virtual void InitAbilityActorInfo();
	
	// Gameplay effect class used to initialize primary attributes (Strength, Intelligence, Resilience, Vigor).
	// Primary attributes are the base stats that directly influence secondary and vital attributes. This effect
	// is applied during character initialization via InitializeDefaultAttributes(). The value for this variable
	// is set in the blueprint, allowing designers to configure different primary attribute values for different
	// character types (e.g., warriors have high Strength, mages have high Intelligence).
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	// Gameplay effect class used to initialize secondary attributes (Armor, ArmorPenetration, BlockChance, etc.).
	// Secondary attributes are derived from primary attributes and affect combat calculations. This effect is
	// applied during character initialization via InitializeDefaultAttributes(), after primary attributes have
	// been set. The value for this variable is set in the blueprint.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	// Gameplay effect class used to initialize vital attributes (Health, Mana, and their maximum values).
	// Vital attributes are the character's resources that are consumed and regenerated during gameplay. This effect
	// is applied during character initialization via InitializeDefaultAttributes(), after primary and secondary
	// attributes have been set, and typically sets these values to their maximums. The value for this variable
	// is set in the blueprint.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	// Applies a gameplay effect to this character at the specified level. This is a helper function used by
	// InitializeDefaultAttributes() to apply the default attribute effects (primary, secondary, and vital).
	// The level parameter allows the same effect class to scale appropriately for characters of different levels.
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	// Initializes all default attributes for this character by applying the gameplay effects stored in
	// DefaultPrimaryAttributes, DefaultSecondaryAttributes, and DefaultVitalAttributes in that specific order.
	// The order matters because secondary attributes depend on primary attributes, and vital attributes depend
	// on both. This function is typically called during InitAbilityActorInfo() after the ability system component
	// has been properly initialized. Virtual to allow child classes to customize attribute initialization.
	virtual void InitializeDefaultAttributes() const;

	// Grants all startup abilities and passive abilities to this character's ability system component.
	// This function iterates through the StartupAbilities array and grants each ability without automatic activation
	// (these are manually activated abilities like attacks or spells). It then iterates through the
	// StartupPassiveAbilities array and grants each ability with automatic activation on grant (these are passive
	// abilities like listening for events ability). This function is typically called during InitAbilityActorInfo()
	// after attributes have been initialized.
	void AddCharacterAbilities();
	
	/* Dissolve Effects */
	
	// Creates a dynamic material instance for dissolve effects out of the values of DissolveMaterialInstance and 
	// WeaponDissolveMaterialInstance and swaps the current material instance on the character's mesh and weapon mesh
	// With these ones
	void Dissolve();
	
	// Takes the dynamic material instance and updates parameters on it. This is implemented in the blueprint
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	
	// Takes the dynamic material instance for the weapon and updates parameters on it. This is implemented in the blueprint
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);
	
	// Variable to store the material instance for dissolve effects for the character's mesh. The value for this variable 
	// is set in the blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;
	
	// Variable to store the material instance for dissolve effects for the character's weapon mesh The value for this 
	// variable is set in the blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
	
	// Variable to store the Niagara system for blood effects. The value for this variable is set in the blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UNiagaraSystem* BloodEffect;
	
	// Variable to store the sound to play when the character dies. The value for this variable is set in the blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USoundBase* DeathSound;
	
	/* Minions */
	
	// Variable to store the number of minions the character has spawned
	int32 MinionCount = 0;
	
	// Variable of the enum type that represents the character class of this enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	
	// Niagara component that displays visual effects when the character has the burn debuff applied. This component
	// is activated/deactivated based on the presence of the burn debuff tag in the character's ability
	// system component. The UDebuffNiagaraComponent class listens for changes to its assigned DebuffTag and controls the
	// Niagara particle system visibility accordingly. The specific Niagara system is set in the blueprint.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;
	
	// Niagara component that displays visual effects when the character has the stun debuff applied. This component
	// is activated/deactivated based on the presence of the stun debuff tag in the character's ability
	// system component. The UDebuffNiagaraComponent class listens for changes to its assigned DebuffTag and controls the
	// Niagara particle system visibility accordingly. The specific Niagara system is set in the blueprint.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
private:
	
	// Array of gameplay abilities that are automatically granted to this character when the ability system
	// is initialized. These are abilities that must be manually activated by the player or AI (e.g., attack abilities,
	// special moves, spells). The abilities in this array are granted during AddCharacterAbilities() and can be
	// activated through input bindings or AI logic. The values for this array are set in the blueprint.
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	// Array of passive gameplay abilities that are automatically granted and activated when the ability system is
	// initialized. Unlike StartupAbilities, these passive abilities are always active and don't require manual
	// activation (e.g., passive stat bonuses, auras, regeneration effects). These abilities are granted during
	// AddCharacterAbilities() with their activation policy set to activate on grant. The values for this array are
	// set in the blueprint.
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;
	
	// Stores the animation montage this class or child classes wish to play. This is likely set in the blueprint
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
};
