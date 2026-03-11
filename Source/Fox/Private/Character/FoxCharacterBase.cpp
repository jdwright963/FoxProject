// Copyright TryingToMakeGames


#include "Character/FoxCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "FoxGameplayTags.h"
#include "AbilitySystem/FoxAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Fox/Fox.h"
#include "Kismet/GameplayStatics.h"

AFoxCharacterBase::AFoxCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Retrieves a const reference to the singleton instance of FFoxGameplayTags, which contains all gameplay tags used
	// throughout the project. Using a reference avoids unnecessary copying and const ensures immutability
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	// Creates a UDebuffNiagaraComponent subobject named "BurnDebuffComponent" that will visually represent the burn
	// debuff effect using a Niagara particle system. This component will automatically activate/deactivate based on
	// whether the character has the burn debuff gameplay tag applied to their ability system component
	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("BurnDebuffComponent");

	// Attaches the burn debuff Niagara component to the character's root component (capsule) so it follows the
	// character's position and rotation, ensuring the particle effect stays with the character as they move
	BurnDebuffComponent->SetupAttachment(GetRootComponent());

	// Assigns the Debuff_Burn gameplay tag to the component's DebuffTag property, which the component uses to register
	// a callback that listens for when this tag is added or removed from the owner's ASC to activate/deactivate the effect
	BurnDebuffComponent->DebuffTag = GameplayTags.Debuff_Burn;
	
	// Configures the capsule component to ignore collision with the camera channel, preventing the camera from being 
	// blocked or pushed by the character's capsule collision during gameplay
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Disables overlap event generation for the capsule component to improve performance by preventing unnecessary 
	// overlap notifications, as overlap detection is instead handled by the skeletal mesh component below
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	// Configures the skeletal mesh to ignore collision with the camera channel, ensuring the camera can freely move 
	// through the character's mesh without being obstructed or pushed away
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// Sets the skeletal mesh to generate overlap events when projectiles (using the ECC_Projectile collision channel) 
	// intersect with it, enabling hit detection for projectile-based attacks without blocking the projectile's movement
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	// Enables overlap event generation for the skeletal mesh component, allowing it to detect and respond to overlaps 
	// with other actors (such as projectiles), which is necessary for hit detection and gameplay interactions
	GetMesh()->SetGenerateOverlapEvents(true);
	
	// Creates a new skeletal mesh component named "Weapon" as a subobject of this character, which will be used to 
	// visually represent and manage the character's equipped weapon (sword, staff, bow, etc.) with its own animations
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");

	// Attaches the weapon skeletal mesh component to a socket named "WeaponHandSocket" on the character's main skeletal 
	// mesh, ensuring the weapon follows the character's hand movements during animations and remains properly positioned
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));

	// Disables all collision detection for the weapon skeletal mesh by default, preventing it from blocking or generating 
	// overlap events until collision is explicitly enabled (typically when the weapon is dropped during the death sequence)
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AFoxCharacterBase::GetAbilitySystemComponent() const
{
	// Returns the character's ability system component.
	return AbilitySystemComponent;
}

UAnimMontage* AFoxCharacterBase::GetHitReactMontage_Implementation()
{
	// Returns the hit reaction animation montage for the character.
	return HitReactMontage;
}

void AFoxCharacterBase::Die(const FVector& DeathImpulse)
{
	// Make the character drop the weapon specifying the detachment rules. 'EDetachmentRule::KeepWorld' keeps the world 
	// transform (the location), and 'true' ensures the change is tracked for the editor's undo system. This is automatically
	// a replicated action. This function runs on the server so we do not need to detatch on clients.
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	
	// Calls the multicast RPC that handles what happens on all clients when the character dies. Passes into this funciton 
	// The DeathImpulse which is a vector representing the force and direction to apply to the character's ragdoll physics upon death.
	MulticastHandleDeath(DeathImpulse);
}

void AFoxCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	// Plays a 3D positional sound effect at a specific world location using Unreal's static gameplay utility class.
	// UGameplayStatics::PlaySoundAtLocation is a fire-and-forget function that spawns a temporary audio component,
	// plays the sound, and then automatically cleans it up when finished. The sound will spatialize (adjust volume
	// and panning based on listener position) for all players who can hear it at the specified location.
	// @param WorldContextObject: (this) Provides the UWorld context needed to spawn the audio component, using this
	//                            character actor as the context object
	// @param Sound: (DeathSound) The USoundBase asset (defined as a member variable) containing the audio data to play,
	//               typically configured in Blueprint to be a death scream, impact sound, or other death-related audio
	// @param Location: (GetActorLocation()) The 3D world position where the sound should originate from, retrieved from
	//                  this character's current location
	// @param Rotation: (GetActorRotation()) The world rotation for directional sounds, retrieved from this character's
	//                  current rotation, though most death sounds are omnidirectional and won't use this parameter
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());
	
	// Enable physics simulation, gravity, and collision for the weapon. This will cause the weapon to fall and bounce
	// around on the ground (ragdoll). The mesh has no collision by default
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// Applies a scaled-down physics impulse to the weapon mesh to simulate it being knocked away during the character's 
	// death. The DeathImpulse vector (passed as a parameter from the Die function) represents the force and direction of 
	// the killing blow, and is multiplied by 0.1 to reduce its magnitude since weapons are typically lighter and should 
	// not fly as far as the character's body. NAME_None indicates the impulse is applied to the entire (root) weapon body rather 
	// than a specific bone, and the 'true' parameter indicates the Strength is taken as a change in velocity instead 
	// of an impulse (ie. mass will have no effect).
	Weapon->AddImpulse(DeathImpulse * 0.1f, NAME_None, true);
	
	// Enable physics simulation, gravity, and collision for the character mesh so that it ragdolls. The mesh has no 
	// collision by default
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// Applies a physics impulse to the character's skeletal mesh to simulate the impact force of the killing blow during 
	// the ragdoll death sequence. The DeathImpulse vector (passed as a parameter from the Die function) contains the full 
	// force and direction calculated based on the damage source and impact location, causing the character's ragdolled body 
	// to fly backwards, spin, or collapse realistically based on where and how they were killed. NAME_None indicates the 
	// impulse is applied to the entire (root) mesh rather than a specific bone, and the 'true' parameter indicates the Strength
	// is taken as a change in velocity instead of an impulse (ie. mass will have no effect).
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	// Sets the collision response for the character mesh to block collisions with WorldStatic objects (walls and floors)
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	// Disable collision for the capsule component of the character so the capsule will not block other characters or 
	// objects from passing through it while this character is dead
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Call the dissolve function to start the dissolve effect
	Dissolve();
	
	// Set the member variable to indicate that the character is dead
	bDead = true;

	// Manually deactivates the burn debuff Niagara component, immediately stopping the burn particle effect on the 
	// character's body. While the component has its own callback registered to deactivate when the Debuff_Burn tag is 
	// removed from the ASC, explicitly calling Deactivate here ensures the visual effect stops instantly on death 
	// regardless of tag state, preventing burn particles from continuing to play on the ragdolled corpse
	BurnDebuffComponent->Deactivate();
	
	// Broadcasts the OnDeathDelegate multicast delegate, notifying all registered listeners that this character has died by 
	// passing 'this' as the dead actor parameter. Listeners can include UI widgets that need to update death counts, 
	// AI controllers that need to select new targets, quest systems tracking enemy kills, debuff components that need 
	// to deactivate visual effects, the electrocute ability to stop since it is an ability that is active as long as the
	// user holds down the input and we want it to stop when the target dies), or any other system that needs to respond to character death events
	OnDeathDelegate.Broadcast(this);
}

void AFoxCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AFoxCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	// Returns the location of the socket on the tip of the character's weapon using the GetSocketLocation function from
	// the weapon component that is inherited from its parent class and passing it the WeaponTipSocketName member variable
	// of this class
	// return Weapon->GetSocketLocation(WeaponTipSocketName);
	
	// Retrieves a const reference to the singleton instance of FFoxGameplayTags, which holds all the gameplay tags
	// defined for the Fox project. Using a reference avoids copying the entire struct and const ensures we cannot 
	// modify the singleton instance
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	// Checks if the provided MontageTag parameter matches exactly the CombatSocket_Weapon tag, indicating this is a
	// weapon-based attack montage. MatchesTagExact ensures an exact match (no partial matches of tag hierarchies).
	// Also validates that the Weapon component pointer is valid before attempting to access it. If both conditions are
	// true, returns the world-space location of the socket at the weapon's tip (defined by WeaponTipSocketName member 
	// variable), which is typically where projectiles should spawn from or where hit detection should occur for weapon attacks
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}

	// Checks if the provided MontageTag parameter matches exactly the CombatSocket_LeftHand tag, indicating this is a
	// left hand-based attack montage (such as a punch or magical effect emanating from the left hand). If true, returns 
	// the world-space location of the socket on the character's skeletal mesh at the left hand position (defined by 
	// LeftHandSocketName member variable), which is where projectiles or effects should spawn from for left hand attacks
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}

	// Checks if the provided MontageTag parameter matches exactly the CombatSocket_RightHand tag, indicating this is a
	// right hand-based attack montage (such as a punch or magical effect emanating from the right hand). If true, returns 
	// the world-space location of the socket on the character's skeletal mesh at the right hand position (defined by 
	// RightHandSocketName member variable), which is where projectiles or effects should spawn from for right hand attacks
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}
	
	// Checks if the provided MontageTag parameter matches exactly the CombatSocket_Tail tag, indicating this is a
	// tail based attack montage. If true, returns 
	// the world-space location of the socket on the character's skeletal mesh at the tail position (defined by 
	// TailSocketName member variable), which is where projectiles or effects should spawn from for tail attacks
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}

	// Default return value if none of the above conditions matched. Returns a zero vector (0, 0, 0) as a fallback,
	// indicating no valid combat socket location could be determined for the provided MontageTag.
	return FVector();
}

bool AFoxCharacterBase::IsDead_Implementation() const
{
	// Returns the value of the bDead member variable to check if the character is dead
	return bDead;
}

AActor* AFoxCharacterBase::GetAvatar_Implementation()
{
	// Returns this instance of FoxCharacterBase
	return this;
}

TArray<FTaggedMontage> AFoxCharacterBase::GetAttackMontages_Implementation()
{
	// Returns the array of attack montages configured for this character
	return AttackMontages;
}

UNiagaraSystem* AFoxCharacterBase::GetBloodEffect_Implementation()
{
	// Returns the Niagara system used to visualize blood effects when this character takes damage
	return BloodEffect;
}

FTaggedMontage AFoxCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	// Iterates through the AttackMontages array (a TArray of FTaggedMontage structs) to search for a montage whose 
	// MontageTag matches the provided MontageTag parameter. This function performs a linear search through all available 
	// attack montages configured for this character to find the specific montage associated with the requested gameplay tag.
	// Uses a range-based for loop which creates a copy of each FTaggedMontage element in the array during iteration. Note 
	// that this creates a copy rather than using a reference, which is acceptable since FTaggedMontage is a small struct 
	// containing only pointers and a tag, but using 'const FTaggedMontage&' would be more efficient for larger structs
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		// Compares the MontageTag member of the current TaggedMontage struct with the MontageTag parameter passed to this 
		// function. The == operator for FGameplayTag performs an exact tag comparison. If this condition evaluates to true, 
		// it means we have found the attack montage that corresponds to the requested gameplay tag (for example, finding 
		// the specific attack animation for a light attack, heavy attack, or special ability attack)
		if (TaggedMontage.MontageTag == MontageTag)
		{
			// Returns the matching TaggedMontage struct immediately upon finding it. The returned struct contains the 
			// animation montage pointer, associated gameplay tags, and impact sound that the caller can use to play the
			// appropriate attack animation and effects
			return TaggedMontage;
		}
	}
	// Default return statement executed only if no matching montage was found after iterating through the entire 
	// AttackMontages array. Returns a default-constructed (empty) FTaggedMontage struct where all pointer members 
	// (Montage, ImpactSound) will be nullptr and tags will be empty. The caller should check if the returned montage 
	// is valid before attempting to use it, as this indicates that the requested MontageTag was not configured in the 
	// character's AttackMontages array
	return FTaggedMontage();
}

int32 AFoxCharacterBase::GetMinionCount_Implementation()
{
	// Returns the current count of minions controlled and/or summoned by this character
	return MinionCount;
}

void AFoxCharacterBase::IncrementMinionCount_Implementation(int32 Amount)
{
	// Increments the minion count by the specified amount
	MinionCount += Amount;
}

ECharacterClass AFoxCharacterBase::GetCharacterClass_Implementation()
{
	// Returns the character's RPG class (Warrior, Elementalist, Ranger, etc.
	return CharacterClass;
}

FOnASCRegistered& AFoxCharacterBase::GetOnASCRegisteredDelegate()
{
	// Returns a reference to the OnAscRegistered multicast delegate, which allows external systems (such as 
	// DebuffNiagaraComponent) to register callbacks that will be invoked when this character's AbilitySystemComponent 
	// is initialized and registered. This is particularly important for components that need to bind to ASC events but 
	// may be created before the ASC itself is ready, allowing them to defer their initialization until the ASC becomes available
	return OnAscRegistered;
}

FOnDeathSignature& AFoxCharacterBase::GetOnDeathDelegate()
{
	// Returns a reference to the OnDeathDelegate multicast delegate, which allows external systems (such as DebuffNiagaraComponent, 
	// UI widgets, AI controllers, or quest systems) to register callbacks that will be invoked when this character dies. 
	// This delegate is broadcast in MulticastHandleDeath_Implementation() with the dying actor as a parameter, enabling 
	// any registered listeners to respond appropriately to the character's death event
	return OnDeathDelegate;
}

USkeletalMeshComponent* AFoxCharacterBase::GetWeapon_Implementation()
{
	// Returns the Weapon member variable (the character's weapon skeletal mesh component)
	return Weapon;
}

void AFoxCharacterBase::InitAbilityActorInfo()
{
	
}


void AFoxCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	
	// Validates that the AbilitySystemComponent pointer is valid (not null and not pending kill). The check macro is 
	// a runtime assertion that will crash the program in development builds if the condition fails, helping catch bugs 
	// early. This ensures we have a valid ASC before attempting to use it for applying gameplay effects
	check(IsValid(GetAbilitySystemComponent()));

	// Validates that the GameplayEffectClass parameter is not null. The check macro ensures we have a valid gameplay 
	// effect class to instantiate before proceeding. Without a valid class, we cannot create an effect spec or apply 
	// any effect, so failing fast here prevents undefined behavior
	check(GameplayEffectClass);

	// Creates a FGameplayEffectContextHandle which is a wrapper around contextual information about how and where a 
	// gameplay effect is being applied. The context stores metadata like the instigator, causer, source object, hit 
	// results, and other relevant data that abilities and effects may need to access during execution. MakeEffectContext() 
	// creates a new empty context from the ASC that can be populated with additional information before creating the effect spec
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();

	// Adds this character (the AFoxCharacterBase instance) as the source object in the gameplay effect context. The 
	// source object represents the actor that originated or is applying the effect, which is useful for gameplay logic 
	// that needs to know who applied an effect (for example, determining damage sources, applying buffs based on the 
	// caster's attributes, or tracking effect ownership for gameplay systems)
	ContextHandle.AddSourceObject(this);

	// Creates a FGameplayEffectSpecHandle which is a handle to a FGameplayEffectSpec - the actual instance of a gameplay 
	// effect that will be applied. MakeOutgoingSpec takes the effect class (template/blueprint), the level at which to 
	// apply it (which scales magnitude calculations), and the context handle (containing metadata about the application). 
	// The resulting spec contains all the calculated values and modifiers ready to be applied to a target's attributes
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);

	// Applies the gameplay effect spec to this character's own AbilitySystemComponent. The function takes a dereferenced 
	// pointer to the spec data (SpecHandle.Data.Get() returns the raw pointer, then * dereferences it) and the target ASC 
	// (in this case, our own ASC since we're applying to self). This executes the effect's logic, modifying attributes, 
	// granting abilities, or applying tags as defined in the GameplayEffect class
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}


void AFoxCharacterBase::InitializeDefaultAttributes() const
{
	// Applies the default primary attributes gameplay effect to this character at level 1. Primary attributes are the 
	// base stats like Strength, Intelligence, Resilience, and Vigor that define the character's core capabilities. 
	// These attributes typically affect secondary attributes through calculations and are the foundation of the 
	// character's attribute system. The level parameter (1.f) determines the magnitude/scaling of the effect
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);

	// Applies the default secondary attributes gameplay effect to this character at level 1. Secondary attributes are 
	// derived stats like Armor, ArmorPenetration, BlockChance, CriticalHitChance, CriticalHitDamage, CriticalHitResistance, 
	// HealthRegeneration, ManaRegeneration, and MaxHealth/MaxMana that are calculated based on the primary attributes. 
	// These attributes represent more specialized combat and survival statistics that change as primary attributes are modified
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);

	// Applies the default vital attributes gameplay effect to this character at level 1. Vital attributes are the 
	// character's current resource values like Health and Mana that are actively consumed and regenerated during gameplay. 
	// This effect typically sets these vital attributes to their maximum values (defined by secondary attributes) when 
	// the character is first initialized, ensuring the character starts at full health and mana
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AFoxCharacterBase::AddCharacterAbilities()
{
	// Performs a checked cast of the base AbilitySystemComponent pointer to the derived UFoxAbilitySystemComponent type.
	// CastChecked is used instead of a regular Cast because we expect this cast to always succeed. The AbilitySystemComponent
	// should always be initialized as a UFoxAbilitySystemComponent (or a class derived from it) in our game architecture.
	// If the cast fails (meaning AbilitySystemComponent is null or not a UFoxAbilitySystemComponent), CastChecked will
	// trigger a fatal error in development builds, helping catch configuration errors early. This is safer than a regular
	// Cast which would silently return nullptr on failure, potentially causing crashes later when we try to call
	// UFoxAbilitySystemComponent specific functions. The FoxASC pointer gives us access to custom functionality defined
	// in our UFoxAbilitySystemComponent class, such as the AddCharacterAbilities function that handles granting abilities
	UFoxAbilitySystemComponent* FoxASC = CastChecked<UFoxAbilitySystemComponent>(AbilitySystemComponent);
	
	// Only the server has authority. So if this check is true then we are on a client and we return early
	if (!HasAuthority()) return;
	
	// Call the function in UFoxAbilitySystemComponent that grants the startup abilities to the owner of the ASC
	FoxASC->AddCharacterAbilities(StartupAbilities);
	
	// Call the function in UFoxAbilitySystemComponent that grants the passive abilities to the owner of the ASC
	FoxASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AFoxCharacterBase::Dissolve()
{
	// Checks if the member variables value is valid
	if (IsValid(DissolveMaterialInstance))
	{
		/*
		 * Creates a Dynamic Material Instance (DMI) at runtime. 
		 * While standard Material Instances are "static" (cannot be changed during gameplay), 
		 * a Dynamic Instance allows the CPU to modify parameters—like a 'Dissolve' slider—while the game is running.
		 * 
		 * @param Parent: (DissolveMaterialInstance) The "template" material or instance we are copying from.
		 * @param InOuter: (this) The owner of this new instance. Setting this to 'this' ensures the 
		 *                 material's lifetime is tied to this Actor and is cleaned up by Garbage Collection.
		*/
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		
		// Applies the Dynamic Material Instance to the material slot 1 (element 0) of the Character
		GetMesh()->SetMaterial(0, DynamicMatInst);
		
		/*
		 * Triggers the Timeline logic to begin the dissolve transition. This is a BlueprintImplementableEvent
		 * we declared.
		 * 
		 * This function initiates a UTimelineComponent, which smoothly interpolates a float value 
		 * over time. As the timeline updates every frame, it will apply those values to the 
		 * provided material, creating the visual "fading away" effect.
		 * 
		 * @param DynamicMatInst: The specific Dynamic Material Instance to be modified. By passing 
		 *                        this as a parameter, the Timeline knows exactly which material's 
		 *                        'Dissolve' parameter it should update during its execution.
		*/
		StartDissolveTimeline(DynamicMatInst);
	}
	
	// Checks if the member variables value is valid
	if (IsValid(WeaponDissolveMaterialInstance))
	{
		// See comments above for very similar code except this one is for the weapon
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);
		StartWeaponDissolveTimeline(DynamicMatInst);
	}
}


