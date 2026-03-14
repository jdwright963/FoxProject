// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UAbilitySystemComponent;
class UNiagaraSystem;
class UAnimMontage;

// Declares a native C++ multicast delegate named FOnASCRegistered that can broadcast to multiple listeners.
// Takes one parameter: UAbilitySystemComponent* which represents the Ability System Component that was registered.
// This is used to notify when an actor's ASC becomes available, allowing components to bind listeners before the ASC exists.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*)

// Declares a dynamic multicast delegate named FOnDeath that can broadcast to multiple listeners in both C++ and Blueprints.
// Takes one parameter: AActor* DeadActor which represents the actor that has died.
// Dynamic delegates are slower than native delegates but can be serialized and used in Blueprints.
// This is used to notify when a character dies so listeners (like DebuffNiagaraComponent) can respond appropriately.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, DeadActor);

// USTRUCT macro declares this as an Unreal struct that can be used in Blueprints (BlueprintType specifier allows it)
// Declares a struct named FTaggedMontage following Unreal's naming convention where structs start with 'F'
// This struct is used to associate an animation montage with a gameplay tag for categorization
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	// Generates required boilerplate code for Unreal's reflection system to work with this struct
	GENERATED_BODY()

	// Pointer to an animation montage asset. Initialized to nullptr to ensure it starts as a null pointer. The value
	// for this variable and the ones below in this struct are set in the editor.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	// A gameplay tag used to identify the montage. FGameplayTag is a struct so no pointer needed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;
	
	// A gameplay tag used to identify the socket for the montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;
	
	// Pointer to an impact sound asset. Initialized to nullptr to ensure it starts as a null pointer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};


// UINTERFACE macro declares this class as an Unreal interface that can be implemented by other classes
// MinimalAPI specifier exports only the minimal API needed for this interface (reduces compile times)
// BlueprintType specifier allows this interface to be used as a type in Blueprint graphs
// UCombatInterface is the UObject wrapper class required by Unreal's reflection system - it doesn't contain the actual interface methods
// The actual interface methods are defined in ICombatInterface below (following Unreal's convention where interface implementations start with 'I')
// Classes that want to implement this interface should inherit from ICombatInterface, not UCombatInterface
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FOX_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// Returns the player level of the character
	// See comment for other BlueprintNativeEvent function below for more details
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();
	
	// Returns the location of the character's combat socket associated with a certain tag. 
	// See comment for other BlueprintNativeEvent function below for more details
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag);
	
	// Function to update the motion warping target. BlueprintImplementableEvents are functions that are meant to be
	// implemented only in blueprints and have no C++ implementation. This is marked BlueprintCallable so it can be
	// called from blueprints as well
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& target);
	
	// BlueprintImplementableEvents functions cannot be overridden in C++ so we choose BlueprintNativeEvent since it can be called
	// in blueprints but also have a C++ implementation. Cannot be marked as virtual. A virtual version that exists in 
	// C++ is auto generated (GetHitReactMontage_Implementation we type this ourselves that is not what we mean by auto
	// generate). We override this in FoxCharacterBase
	// Function that returns the hit reaction animation montage of the class that implements this interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();
	
	// Pure virtual function that handles character death. Takes a DeathImpulse parameter which is a vector representing
	// the force and direction to apply to the character's ragdoll physics upon death. The = 0 syntax makes this a pure 
	// virtual function, which means any class that implements this interface MUST override this function with their own 
	// implementation. Unlike the other virtual functions in this interface that have default implementations, this cannot 
	// be left unimplemented. This is marked as virtual (unlike BlueprintNativeEvent functions) and can be overridden 
	// normally in C++. This is used to handle character death logic specific to each character type that implements this interface
	virtual void Die(const FVector& DeathImpulse) = 0;
	
	// Function that returns whether the character is dead or not
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;

	// Function that returns the actor that implements this interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();
	
	// Function that returns an array of attack animation montages with their associated gameplay tags. 
	// BlueprintNativeEvent allows this to be overridden in both C++ (_Implementation version) and
	// Blueprints, enabling different characters to define their own unique attack montage sets
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();
	
	// Function to return the variables value that stores the Niagara system for blood effects. The value is set in the
	// editor
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetBloodEffect();
	
	// Gets an instance of FTaggedMontage with a specific MontageTag from the AttackMontages array
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);
	
	// Function to return the number of minions the character has spawned
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();
	
	// Function to increment the minion count by a specified amount
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementMinionCount(int32 Amount);
	
	// Function to return the character's RPG class (not the C++ class)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ECharacterClass GetCharacterClass();
	
	/** 
	 * Pure virtual function that returns a reference to the FOnASCRegistered multicast delegate. This delegate is 
	 * broadcast when the character's Ability System Component (ASC) becomes available. Components like 
	 * DebuffNiagaraComponent use this to bind listeners before the ASC exists, allowing them to register for 
	 * gameplay tag events as soon as the ASC is ready. The = 0 syntax makes this a pure virtual function requiring
	 * implementation in derived classes. Marked as virtual (not BlueprintNativeEvent) for normal C++ override.
	 *
	 * Why return a delegate reference via an interface function instead of direct member access?
	 * This approach provides abstraction and follows the Dependency Inversion Principle, ensuring the 
	 * component (e.g., DebuffNiagaraComponent) remains decoupled from concrete character classes. 
	 *
	 * Instead of including specific headers (like FoxCharacter.h) in the class where we want to bind to this delegate, 
	 * which leads to tight coupling, rigid code, and circular dependencies, the component only needs to know about the 
	 * ICombatInterface. This allows the component to work polymorphically with any actor (Players, AI, Bosses) that 
	 * implements the interface. It ensures architectural separation where high-level logic depends 
	 * on abstractions rather than specific implementations, keeping the codebase flexible and maintainable.
	*/
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() = 0;

	/** 
	 * Pure virtual function that returns a reference to the FOnDeathSignature dynamic multicast delegate. This delegate is
	 * broadcast when the character dies, allowing listeners (like DebuffNiagaraComponent) to respond to death events
	 * by performing cleanup or state changes (e.g., deactivating visual effects). The = 0 syntax makes this a pure
	 * virtual function requiring implementation in derived classes. Marked as virtual (not BlueprintNativeEvent) for
	 * normal C++ override. Being a dynamic delegate, it can be bound to both C++ and Blueprint functions.
	 *
	 * Why return a delegate reference via an interface function instead of direct member access?
	 * This approach provides abstraction and follows the Dependency Inversion Principle, ensuring the 
	 * component (e.g., DebuffNiagaraComponent) remains decoupled from concrete character classes. 
	 *
	 * Instead of including specific headers (like FoxCharacter.h) in the class where we want to bind to this delegate, 
	 * which leads to tight coupling, rigid code, and circular dependencies—the component only needs to know about the 
	 * ICombatInterface. This allows the component to work polymorphically with any actor (Players, AI, Bosses) that 
	 * implements the interface. It ensures architectural separation where high-level logic depends 
	 * on abstractions rather than specific implementations, keeping the codebase flexible and maintainable.
	*/
	virtual FOnDeathSignature& GetOnDeathDelegate() = 0;
	
	// Function we implement in BP_FoxCharacter. This function sets the value of the InShockLoop variable there, which 
	// indicates whether the Electrocute ability is active, since this is an ability that is active as long as the input
	// that activates it is held down.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetInShockLoop(bool bInLoop);
	
	// Function that returns the skeletal mesh component representing the character's weapon. BlueprintNativeEvent allows
	// this to be overridden in both C++ (_Implementation version) and Blueprints, enabling different characters to
	// return their specific weapon mesh components. This is used by abilities (like beam spells) to get socket locations
	// on the weapon mesh for spawning projectiles or effects from the correct position (e.g., staff tip, gun barrel).
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetWeapon();
	
	// Function that returns whether the character is currently being shocked/electrocuted. BlueprintNativeEvent allows
	// this to be overridden in both C++ (_Implementation version) and Blueprints. This is used to check if the character
	// is being shocked by the Electrocute ability, allowing systems to query the shocked state (e.g., for animation states,
	// AI behavior, or visual effects). The const specifier indicates this function doesn't modify the object's state.
	// We do not implement this function in this class, only in child classes.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsBeingShocked() const;

	// Function that sets whether the character is currently being shocked/electrocuted. BlueprintNativeEvent allows
	// this to be overridden in both C++ (_Implementation version) and Blueprints. Takes a bInShock parameter which
	// is a boolean indicating whether to enable (true) or disable (false) the shocked state. This is used by the
	// Electrocute ability to mark characters as being shocked by the Electrocute ability, enabling appropriate
	// visual effects, animations, and gameplay responses. We do not implement this function in this class, only in 
	// child classes.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetIsBeingShocked(bool bInShock);
};
