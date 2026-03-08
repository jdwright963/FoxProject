// Copyright TryingToMakeGames


#include "AbilitySystem/FoxAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "FoxAbilityTypes.h"
#include "FoxGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/FoxAbilitySystemLibrary.h"
#include "Fox/FoxLogChannels.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/FoxPlayerController.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UFoxAttributeSet::UFoxAttributeSet()
{
	//Gets the singleton instance of the FFoxGameplayTags class that contains the gameplay tags
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();
	
	/* Primary Attributes */
	
	// Adds the Strength gameplay attribute getter function to the TagsToAttributes map, associating the Strength 
	// gameplay tag (key) with its corresponding attribute getter function (value). So that when the map is searched for
	// a key (tag for an attribute) the corresponding value is the getter function for that attribute which returns
	// the FGameplayAttribute for that attribute. This is able to happen because the map takes a function pointer as the 
	// value
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);
	
	/* Secondary Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
	
	/* Resistance Attributes */
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void UFoxAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Primary Attributes
	
	// Step 4 to adding attributes
	
	/*
		DOREPLIFETIME_CONDITION_NOTIFY - Comprehensive Replication Macro for Gameplay Attributes:

		This macro registers the Strength attribute for network replication with full control over replication
		conditions and notification behavior. It combines property registration, conditional replication rules,
		and RepNotify callback control in a single macro call, streamlining the replication setup process.

		Parameters Breakdown:
		1. UFoxAttributeSet - Specifies the class that contains the property being replicated. This tells the
		   replication system which AttributeSet class owns this attribute.

		2. Strength - The exact property name (member variable) within UFoxAttributeSet that should be replicated
		   across the network. Must match the UPROPERTY name declared in the header file.

		3. COND_None - The replication condition flag that determines WHEN replication occurs:
		   - COND_None: Replicate always to all connected clients without any restrictions
		   - Other options include: COND_OwnerOnly (only to owning client), COND_SkipOwner (to everyone except
		     owner), COND_InitialOnly (only during initial replication), COND_SimulatedOnly, etc.
		   - Using COND_None ensures every client receives Strength updates regardless of ownership or connection state

		4. REPNOTIFY_Always - Controls when the RepNotify callback function (OnRep_Strength) executes:
		   - REPNOTIFY_Always: The callback fires EVERY time the value replicates from server to client, even if
		     the new value is identical to the old value
		   - Alternative is REPNOTIFY_OnChanged: Callback only fires when the replicated value actually differs
		     from the previous value
		   - REPNOTIFY_Always is crucial for GAS because it ensures proper prediction reconciliation, allows
		     GAMEPLAYATTRIBUTE_REPNOTIFY to update attribute aggregators consistently, and guarantees that
		     OnAttributeChange delegates broadcast reliably to update UI and gameplay systems

		Gameplay Ability System Context:
		In GAS, attributes like Strength can be modified through GameplayEffects which may apply temporary modifiers,
		buffs, debuffs, or instant changes. The REPNOTIFY_Always flag ensures that:
		- Client-side prediction corrections happen properly when server authority overrides predicted values
		- Attribute aggregators (which calculate final values from base + modifiers) stay synchronized
		- UI elements and gameplay systems receive consistent update notifications
		- Temporary modifiers that don't change the final value still trigger proper system updates

		This setup maintains network consistency between server authority and client predictions in multiplayer
		scenarios where Strength might be modified by equipment changes, level-ups, buffs, debuffs, or other
		GameplayEffects, ensuring all clients see accurate and synchronized attribute values.
	*/
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	// Secondary Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	
	// Resistance Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
	
	// Vital Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UFoxAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

/*
 * Is called right before any attribute changes. Epic recommends we only use this function to do clamping and no other gameplay
 * logic. Later operations recalculate the current value from all modifiers and we will need to clamp again
 * PostGameplayEffectExecute is a better place to clamp
*/
void UFoxAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		// Changes the new value to one that cannot be more than the max health or less than 0
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		// Changes the new value to one that cannot be more than the max mana or less than 0
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

/**
	 * SetEffectProperties - Populates Effect Property Data from Gameplay Effect Callback:
	 * 
	 * This helper function extracts and organizes all relevant actor, component, and controller information
	 * from a gameplay effect execution into a structured FEffectProperties for easy access. It's typically
	 * called at the beginning of PostGameplayEffectExecute to set up both source (instigator) and target
	 * (receiver) information needed for attribute modification logic, damage calculations, or other
	 * gameplay effect responses.
	 * 
	 * @param Data - const FGameplayEffectModCallbackData& - The callback data structure provided by the
	 * Gameplay Ability System when a gameplay effect executes and modifies an attribute. Contains the
	 * FGameplayEffectContextHandle with source/target information, the FGameplayEffectSpec with effect
	 * configuration, and the EvaluatedData with the actual attribute being modified and its magnitude.
	 * This parameter is const reference since we only read from it to populate Props.
	 * 
	 * @param Props - FEffectProperties& - A reference to the FEffectProperties struct that will be populated
	 * with extracted information. This struct will be filled with the source AbilitySystemComponent, avatar
	 * actor, controller, and character, as well as the corresponding target versions of these objects. Passed
	 * by non-const reference so the function can modify it and populate all member variables. After this
	 * function executes, Props will contain all the context needed to implement gameplay logic based on who
	 * applied the effect and who received it.
	*/
// Function to set the values in the FEffectProperties struct
void UFoxAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
	// Source = causer of the effect, Target = target of the effect (owner of this attribute set)
	
	// Retrieves the FGameplayEffectContextHandle from the FGameplayEffectSpec contained in the Data input parameter,
	// which stores contextual information about who or what caused this effect, when it was applied, and other metadata
	// needed for proper effect execution
	Props.EffectContextHandle = Data.EffectSpec.GetContext();

	// Gets the UAbilitySystemComponent of the original instigator (the entity that initially caused this effect) from the 
	// context handle, which allows us to access the source's ability system and determine who actually triggered this effect
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	// Validates that the source ability system component exists and that its AbilityActorInfo struct (containing actor 
	// references) and the AvatarActor (the visual representation of the ability owner) within it are all valid before 
	// attempting to access them, preventing null pointer crashes
	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		// Retrieves and stores the source's avatar actor (the actual in-game actor representing the effect instigator, 
		// such as a character or projectile) by getting the raw pointer from the TWeakObjectPtr stored in AbilityActorInfo
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();

		// Retrieves and stores the player controller associated with the source from AbilityActorInfo, which will be valid 
		// for player-controlled characters but nullptr for AI-controlled characters or non-pawn actors
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();

		// Checks if the source controller is null (meaning it wasn't set in AbilityActorInfo, typically for AI characters) 
		// while the source avatar actor is valid, indicating we need to attempt an alternative method to get the controller
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			// Attempts to cast the source avatar actor to APawn, and if successful (meaning the source is a pawn-type actor 
			// like a character), stores it in a const local variable for safe access within this scope
			if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				// Retrieves the controller from the pawn using GetController(), which works for both player-controlled and 
				// AI-controlled pawns, ensuring we have a valid controller reference even when AbilityActorInfo didn't provide one
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

// Executed after a gameplay effect changes an attribute
void UFoxAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// Create an instance of the FEffectProperties struct we defined
	FEffectProperties Props;
	
	// Populates the values in the FEffectProperties Props struct with values from
	// FGameplayEffectModCallbackData& - The callback data structure provided by the
	// Gameplay Ability System when a gameplay effect executes and modifies an attribute.
	SetEffectProperties(Data, Props);
	
	/**
	 * Early return guard clause that prevents applying gameplay effects to already-dead characters.
	 * 
	 * This check performs two validations before processing any attribute modifications:
	 * 1. Props.TargetCharacter->Implements<UCombatInterface>() verifies the target implements the combat interface
	 * 2. ICombatInterface::Execute_IsDead(Props.TargetCharacter) checks if the target's death flag is set to true
	 * 
	 * If both conditions are true (target is a combat-capable character AND is already dead), the function returns
	 * immediately without processing the gameplay effect. This prevents several issues:
	 * - Applying damage/healing to corpses that should no longer be interactive
	 * - Triggering hit reactions on dead characters (preventing animation glitches)
	 * - Processing debuffs on targets that have already triggered death logic
	 * - Awarding duplicate XP if multiple damage effects arrive after death
	 * - Showing floating damage numbers above corpses
	*/
	if(Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter)) return;
	
	// Check if the attribute being modified is the health attribute
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Retrieves the Health attribute's current value (which has ALREADY been modified by the gameplay effect),
		// clamps that modified value between 0 and MaxHealth using FMath::Clamp() to prevent invalid states
		// (negative health or exceeding maximum), then writes the clamped result back to the Health attribute
		// using SetHealth() to ensure the attribute stays within valid bounds after the gameplay effect application
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	// Check if the attribute being modified is the mana attribute
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Retrieves the Mana attribute's current value (which has ALREADY been modified by the gameplay effect),
		// clamps that modified value between 0 and MaxMana using FMath::Clamp() to prevent invalid states
		// (negative mana or exceeding maximum), then writes the clamped result back to the Mana attribute
		// using SetMana() to ensure the attribute stays within valid bounds after the gameplay effect application
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}
	// Check if the attribute being modified is the IncomingDamage attribute
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// Processes the incoming damage by subtracting it from the target's health, checking for fatal damage to
		// trigger death logic and experience point rewards, activating hit reaction abilities for non-fatal hits, displaying floating damage
		// text to relevant player controllers, and applying any associated debuff effects if the damage successfully
		// applied a debuff
		HandleIncomingDamage(Props);
	}
	// Check if the attribute being modified is the IncomingXP attribute
	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		// Processes the incoming experience points by extracting the XP amount, resetting the IncomingXP meta attribute,
		// calculating level-ups and rewards, distributing attribute/spell points, and updating the player's total XP
		HandleIncomingXP(Props);
	}
}

void UFoxAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	// After storing the value of the IncomingDamage reset the IncomingDamage attribute to 0, so that it can be used
	// again the next time this function is called when a gameplay effect is applied
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	
	// Check if this variable is greater than 0
	if (LocalIncomingDamage > 0.f)
	{
		// Calculate the new health value after applying the incoming damage
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		
		// Set the health attribute to the new value, clamped between 0 and the maximum health
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
		
		// Creates a boolean flag to indicating if the incoming damage was fatal
		const bool bFatal = NewHealth <= 0.f;
		
		// Check if the damage was fatal
		if (bFatal)
		{
			// Casts the TargetAvatarActor from the FEffectProperties Props struct to type ICombatInterface
			ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
			
			// Checks if the cast was successful
			if (CombatInterface)
			{
				// Retrieves the death impulse vector from the gameplay effect context handle, which represents the
				// directional force that should be applied to the dying character's ragdoll physics body.
				// The GetDeathImpulse() utility function extracts this vector from the custom FFoxGameplayEffectContext
				FVector Impulse = UFoxAbilitySystemLibrary::GetDeathImpulse(Props.EffectContextHandle);

				/**
				 * Calls the Die() function on the CombatInterface, passing the death impulse vector as a parameter.
				 * The Impulse parameter is passed to the Die() function to provide the death impulse vector that
				 * will be applied to the character's ragdoll physics. This vector contains both the direction and
				 * magnitude of force that should launch or push the dying character, creating realistic death physics
				 * reactions. For example, a powerful critical hit might send an enemy flying backward with a large
				 * impulse magnitude, while a weak finishing blow might result in a small impulse causing the character
				 * to simply collapse.
				 * 
				 * Due to polymorphism and virtual function dispatch, this call does not invoke the empty base
				 * implementation in ICombatInterface. Instead, it calls the overridden Die() implementation
				 * in the actual runtime type of the object (either AFoxCharacterBase or AFoxEnemy, depending
				 * on what TargetAvatarActor actually is). Both classes inherit from ICombatInterface and
				 * provide their own Die() implementations. When Die() is called through the interface pointer,
				 * C++ uses virtual function dispatch to automatically route the call to the correct derived
				 * class implementation, ensuring each character type executes its specific death behavior
				 * (AFoxCharacterBase::Die() or AFoxEnemy::Die()).
				 * 
				 * This works because the CPU performs a "VTable (Virtual Method Table) lookup" at runtime, finding 
				 * the specific memory address for the version of Die() belonging to the actual object. An interface 
				 * is used here instead of a base class to allow for "decoupling", this ensures that 
				 * vastly different objects (like a Player, a Dragon, or a Destructible Crate) can all 
				 * support dying without being forced to inherit from the same parent class.
				 * 
				 * This is known as "Runtime Dispatch". The exact function call isn't hard-coded into the 
				 * binary instructions at compile time. Instead, the CPU looks up the correct function 
				 * address via the VTable while the game is running. Using an interface is preferable 
				 * here over a shared base class because it allows unrelated objects to share behavior, 
				 * while a Warrior and an Enemy might share a parent class, a Destructible Crate or 
				 * Explosive Barrel are completely different types that can still implement 
				 * ICombatInterface to handle their own death logic.
				*/
				CombatInterface->Die(Impulse);
			}
			/**
			 * Sends an experience point reward event to the source character (the killer) after a fatal hit.
			 * 
			 * This function call triggers the XP distribution system by sending a gameplay event with the
			 * Attributes_Meta_IncomingXP tag to the source character's ability system component. The function
			 * calculates the appropriate XP reward based on the target's character class and level, packages
			 * this information into a FGameplayEventData payload, and sends it to any abilities on the
			 * source character that are listening for XP gain events (such as the passive ability GA_ListenForEvent).
			*/
			SendXPEvent(Props);
		}
		// Check if the damage was not fatal
		else
		{
			// Creates a tag container
			FGameplayTagContainer TagContainer;
			
			// Gets the singleton instance of FFoxGameplayTags and from it gets the Effects_HitReact tag and adds it
			// to the tag container
			TagContainer.AddTag(FFoxGameplayTags::Get().Effects_HitReact);
			
			// Try activating abilities by tag on the target ASC. The ability must be given this ability tag in the
			// editor in the details panel. In the GA_HitReact BP we add the Debuff.Burn tag to the Activation Blocked Tags
			// in the details panel to prevent the hit react animation from playing when the enemy's ASC has the Debuff.Burn tag,
			// because the hit react animation does not allow the enemy to move and we want them to continue chasing their
			// target while burning.
			Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
			
			/**
			 * Retrieves the knockback force vector from the gameplay effect context handle using the custom
			 * GetKnockbackForce() utility function, which extracts the directional force that should be applied to
			 * the target character to create a physics-based knockback effect. This vector contains both the direction 
			 * and magnitude of the knockback force. The vector's direction determines which way the character will be pushed
			 * (away from attacker, upward, etc.), while its magnitude determines the strength of the knockback effect.
			 * Stored as a const reference to avoid unnecessary copying of the FVector data.
			*/
			const FVector& KnockbackForce = UFoxAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);

			/**
			 * Checks if the knockback force vector has a magnitude greater than 1.0 using IsNearlyZero with a tolerance
			 * of 1.f, which prevents attempting to launch the character with a zero or near-zero force vector that would
			 * have no visible effect. This validation is necessary because not all damaging abilities apply knockback
			 * (some may have KnockbackForce set to FVector::ZeroVector), and attempting to launch a character with zero
			 * force would be wasteful and could potentially cause unexpected physics behavior. The tolerance value of 1.f
			 * means any knockback force with magnitude less than 1 unit will be ignored.
			*/
			if (!KnockbackForce.IsNearlyZero(1.f))
			{
				/**
				 * Applies the knockback force to the target character by calling LaunchCharacter (a system defined function
				 * in Character.cpp), which overrides the character's current velocity with the provided force vector and 
				 * enables physics simulation to create a knockback effect. The first boolean parameter (true) tells 
				 * LaunchCharacter to override the XY (horizontal) velocity components, replacing the character's current 
				 * movement with the knockback force's horizontal direction.
				 * 
				 * The second boolean parameter (true) tells LaunchCharacter to override the Z (vertical) velocity component, 
				 * allowing the knockback to launch the character upward if the force vector has a positive Z component. 
				 * With both parameters set to true, the character's entire velocity is replaced by the knockback force,
				 * creating a dramatic physics-driven knockback effect where the character is sent flying in the direction 
				 * specified by the KnockbackForce vector.
				*/
				Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			}
			
			/**
			 * Checks if the gameplay effect that caused this damage successfully applied a debuff to the target.
			 * 
			 * This conditional uses the IsSuccessfulDebuff() utility function to query the FGameplayEffectContextHandle
			 * stored in Props, which contains metadata about the effect execution including whether a debuff condition
			 * was met (such as passing a chance-based roll). If the function returns
			 * true, indicating a debuff was successfully applied, we proceed to execute debuff-specific logic. If false,
			 * the damage was dealt without any associated debuff effect and we skip the debuff processing entirely.
			*/
			if (UFoxAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
			{
				// Calls the Debuff helper function to dynamically create and apply a gameplay effect that inflicts a 
				// damage-over-time debuff on the target character, using the damage type from the original effect's context
				// to determine which specific debuff should be applied (burn for fire, stun for lightning, etc.)
				Debuff(Props);
			}
		}
		// Use the UFoxAbilitySystemLibrary getter functions for bBlockedHit and bCriticalHit to get their values
		// passing in the gameplay effect context handle from the Props struct
		const bool bBlock = UFoxAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		const bool bCriticalHit = UFoxAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		
		/**
		 * Calls ShowFloatingText to display floating damage numbers above the target character for visual feedback.
		 * 
		 * This function determines which player controller (if any) should see the damage numbers based on whether
		 * the source (attacker) or target (victim) is player-controlled, then spawns a damage text widget at the
		 * target's location. The function handles all multiplayer scenarios: player vs player (attacker sees damage),
		 * player vs AI (attacker sees damage), AI vs player (victim sees damage), and AI vs AI (no display).
		 * 
		 * Parameters:
		 * - Props: FEffectProperties struct containing source and target actor/controller/ASC references
		 * - LocalIncomingDamage: The calculated damage amount to display as floating text
		 * - bBlock: Boolean flag indicating if the hit was blocked, used to modify visual appearance
		 * - bCriticalHit: Boolean flag indicating if this was a critical hit, used to show special styling
		*/
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);
	}
}

void UFoxAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	// After storing the value of the IncomingXP reset the IncomingXP attribute to 0, so that it can be used
	// again the next time this function is called when a gameplay effect is applied
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0.f);
	
	/*
	 * Checks if the source character implements both UPlayerInterface (for XP/leveling systems) and
	 * UCombatInterface (for level retrieval). Only player-controlled characters like AFoxCharacter
	 * implement both interfaces, while AI enemies only implement UCombatInterface. This prevents
	 * crashes when accessing player-specific functions on non-player characters.
	*/
	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		// Retrieves the player's current level by calling GetPlayerLevel() through the ICombatInterface using the
		// Execute_ wrapper function. This returns the player's level before any XP is added, which we need as a
		// baseline to calculate how many levels the player will gain from the incoming XP reward
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);

		// Retrieves the player's current accumulated experience points by calling GetXP() through the IPlayerInterface
		// using the Execute_ wrapper function. This returns the total XP the player has earned so far (before adding
		// the new XP from this enemy kill), which we need to calculate whether the player has earned enough total XP
		// to advance to a new level
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

		// Calculates what level the player should be at after adding the incoming XP reward to their current XP total
		// by calling FindLevelForXP() through the IPlayerInterface. This function looks up the appropriate level in a
		// data table based on the total XP amount (CurrentXP + LocalIncomingXP), returning the highest level whose XP
		// requirement the player has met or exceeded
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);

		// Calculates how many levels the player will gain by subtracting their current level from the newly calculated
		// level. If the player had enough XP to advance multiple levels (e.g., from level 5 to level 8), NumLevelUps
		// would be 3. If the XP gain wasn't enough to level up at all, NumLevelUps will be 0 and the level-up logic
		// below will be skipped
		const int32 NumLevelUps = NewLevel - CurrentLevel;
		
		// Checks if the player gained at least one level from the incoming XP reward. If NumLevelUps is 0, the player
		// gained XP but didn't have enough to advance to the next level, so we skip all the level-up logic below.
		// If NumLevelUps is greater than 0, the player leveled up and should receive rewards
		if (NumLevelUps > 0)
		{
			// Retrieves the number of attribute points the player should be awarded for reaching their current level
			// by calling GetAttributePointsReward() through the IPlayerInterface. This function looks up the reward
			// value in the LevelUpInfo data asset based on the CurrentLevel parameter, returning how many attribute
			// points the player earned for this specific level advancement
			const int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel);

			// Retrieves the number of spell points the player should be awarded for reaching their current level by
			// calling GetSpellPointsReward() through the IPlayerInterface. This function looks up the reward value
			// in the LevelUpInfo data asset based on the CurrentLevel parameter, returning how many spell points
			// the player earned for this specific level advancement
			const int32 SpellPointsReward = IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel);

			// Increases the player's level by the number of levels gained (NumLevelUps) by calling AddToPlayerLevel()
			// through the IPlayerInterface. This updates the player's stored level value in the PlayerState, which
			// is used for stat calculations. If the player gained multiple levels from a single XP reward, all
			// levels are added at once
			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);

			// Adds the calculated attribute point reward to the player's available attribute points by calling
			// AddToAttributePoints() through the IPlayerInterface. These points can be spent by the player to
			// permanently increase primary attributes like Strength, Intelligence, Resilience, or Vigor through
			// the character progression UI
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);

			// Adds the calculated spell point reward to the player's available spell points by calling
			// AddToSpellPoints() through the IPlayerInterface. These points can be spent by the player to
			// unlock new abilities or upgrade existing abilities through the spell progression UI
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);
			
			// Sets a flag that triggers health restoration to maximum in PostAttributeChange() when MaxHealth
			// increases from primary attribute upgrades gained during level-up, ensuring the player's current
			// health scales proportionally with their new maximum health value rather than remaining at the old value
			bTopOffHealth = true;

			// Sets a flag that triggers mana restoration to maximum in PostAttributeChange() when MaxMana increases
			// from primary attribute upgrades gained during level-up, ensuring the player's current mana scales
			// proportionally with their new maximum mana value rather than remaining at the old value
			bTopOffMana = true;

			// Calls the LevelUp() function through the IPlayerInterface to trigger any additional level-up logic
			// such as broadcasting delegates to update UI elements, playing level-up visual effects or sounds,
			// displaying level-up notifications, or saving the player's new level to persistent storage. This
			// serves as the final notification that the level-up process is complete
			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
		}
		
		/**
		 * Calls the AddToXP function on the source character through the IPlayerInterface interface.
		 * 
		 * The Execute_ prefix is Unreal's auto-generated C++ way to call BlueprintNativeEvent functions.
		 * Execute_ functions check if Blueprint overrides exist, if not they call the _Implementation version
		 * First parameter (Props.SourceCharacter) is the UObject implementing the interface
		 * Second parameter (LocalIncomingXP) is the XP amount to add
		 * 
		 * XP FLOW THROUGH THE SYSTEM:
		 * 1. Enemy dies and grants XP reward (calculated earlier in this function)
		 * 2. AttributeSet calls AddToXP() through IPlayerInterface (HERE - this line)
		 * 3. Call routes to AFoxCharacter::AddToXP_Implementation() (the middleman function)
		 * 4. AFoxCharacter forwards XP to AFoxPlayerState::AddToXP() (different function with same name)
		 * 5. PlayerState accumulates XP, checks for level-up, broadcasts UI updates
		 * 
		 * WHY Execute_ FUNCTIONS WORK LIKE STATIC FUNCTIONS (BUT AREN'T):
		 * 
		 * When you declare a BlueprintNativeEvent in an interface (UFUNCTION(BlueprintNativeEvent)), Unreal's
		 * Header Tool (UHT) automatically generates an Execute_ function during the build process. This generated
		 * function is a TRUE STATIC FUNCTION that gets created in the interface class, which is why we can call
		 * it using the scope resolution operator (IPlayerInterface::Execute_AddToXP) without needing an instance.
		 * 
		 * The Execute_ function signature that UHT generates looks like this:
		 * static void Execute_AddToXP(UObject* Object, int32 InXP);
		 * 
		 * Notice it takes the UObject (implementing the interface) as the FIRST PARAMETER rather than being
		 * called ON an object instance. This is why the syntax IPlayerInterface::Execute_AddToXP(SourceCharacter, XP)
		 * works - it's calling a static helper function that internally performs these steps:
		 * 
		 * 1. Checks if the passed UObject has a Blueprint override of AddToXP using reflection
		 * 2. If Blueprint override exists: Calls the Blueprint version via the Blueprint VM
		 * 3. If no Blueprint override: Calls the C++ _Implementation version (AddToXP_Implementation)
		 * 4. If no _Implementation exists: Falls back to the interface's default implementation if any
		 * 
		 * This design pattern allows BlueprintNativeEvent functions to support three implementation options:
		 * - Pure C++ implementation (just write _Implementation version)
		 * - Pure Blueprint implementation (override in Blueprint, no _Implementation needed)
		 * - Hybrid (C++ _Implementation as default, optionally overridden in specific Blueprint classes)
		 * 
		 * The static Execute_ wrapper handles all the complexity of checking which version to call, making
		 * it safe to call from any context without manually checking for Blueprint overrides or worrying
		 * about virtual function dispatch. You COULD call AddToXP_Implementation directly, but then you'd
		 * skip any Blueprint overrides and miss the flexibility that BlueprintNativeEvent provides.
		*/
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void UFoxAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	/**
	 * Restores current Health to maximum when MaxHealth increases during level-up attribute gains.
	 * 
	 * When a player levels up and spends attribute points to increase primary attributes like Vigor,
	 * the MaxHealth secondary attribute recalculates through the MMC_MaxHealth modifier magnitude calculation
	 * and increases accordingly. Without this restoration logic, the player's current Health would remain at
	 * its old value (e.g., 150/200 health becomes 150/250 after leveling), leaving them proportionally weaker.
	 * By setting current Health equal to the new MaxHealth value, we ensure the player receives the full benefit
	 * of their level-up by starting at full health with their new maximum, making level-ups feel rewarding and
	 * preventing players from needing to heal after gaining levels. The bTopOffHealth flag is set to true in
	 * PostGameplayEffectExecute during level-up processing to signal that this restoration should occur.
	*/
	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		// Sets current Health to the new MaxHealth value, effectively healing the player to full health after a level-up
		SetHealth(GetMaxHealth());
		// Resets the flag to false after restoration to prevent unintended health refills on future MaxHealth changes
		bTopOffHealth = false;
	}
	/**
	 * Restores current Mana to maximum when MaxMana increases during level-up attribute gains.
	 * 
	 * When a player levels up and spends attribute points to increase primary attributes like Intelligence,
	 * the MaxMana secondary attribute recalculates through the MMC_MaxMana modifier magnitude calculation
	 * and increases accordingly. Without this restoration logic, the player's current Mana would remain at
	 * its old value (e.g., 100/150 mana becomes 100/200 after leveling), leaving them unable to immediately
	 * use their expanded mana pool. By setting current Mana equal to the new MaxMana value, we ensure the
	 * player receives the full benefit of their level-up by starting at full mana with their new maximum,
	 * making level-ups feel rewarding and allowing immediate access to higher-cost abilities. The bTopOffMana
	 * flag is set to true in PostGameplayEffectExecute during level-up processing to signal that this
	 * restoration should occur.
	*/
	if (Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		// Sets current Mana to the new MaxMana value, effectively restoring the player to full mana after a level-up
		SetMana(GetMaxMana());
		// Resets the flag to false after restoration to prevent unintended mana refills on future MaxMana changes
		bTopOffMana = false;
	}
}

void UFoxAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const
{
	// Checks if the source and target characters are different, because we do not want to show damage numbers 
	// for damage caused to self
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		// Gets the player controller for the source character and casts it to AFoxPlayerController to
		// access the ShowDamageNumber function. Returns nullptr if not a valid AFoxPlayerController (e.g., AI)
		
		
		/*
		 * Damage Number Display Logic - Determines Which Player Controller Should See Damage Feedback:
		 * 
		 * This section implements a priority-based system for displaying damage numbers to player controllers.
		 * The logic follows a specific order to ensure damage feedback is shown to the most relevant player:
		 * 
		 * Priority 1: Show to the SOURCE player (the attacker who caused the damage)
		 *   - If the source character is controlled by a player controller, that player sees the damage they dealt
		 *   - This provides immediate feedback to attacking players about their damage output
		 *   - If successful, we return early to avoid showing duplicate damage numbers
		 * 
		 * Priority 2: Show to the TARGET player (the victim receiving the damage)
		 *   - Only executes if Priority 1 failed (source is AI or not a player controller)
		 *   - If the target character is controlled by a player controller, that player sees the damage they received
		 *   - This provides feedback to defending players about incoming damage
		 * 
		 * This system handles all multiplayer scenarios:
		 * - Player attacks Player: Attacker sees damage (Priority 1)
		 * - Player attacks AI: Attacker sees damage (Priority 1)
		 * - AI attacks Player: Victim sees damage (Priority 2)
		 * - AI attacks AI: No damage numbers shown (both checks fail, which is correct)
		 * 
		 * The early return is crucial to prevent showing damage numbers twice when both source and target
		 * are player-controlled (e.g., PvP scenarios).
		*/

		// Attempts to cast the source character's controller to AFoxPlayerController to check if the attacker
		// is controlled by a human player (as opposed to AI). If successful, PC will contain a valid pointer
		// to the player controller; if the cast fails (source is AI-controlled), PC will be nullptr and this
		// block will be skipped.
		if (AFoxPlayerController* PC = Cast<AFoxPlayerController>(Props.SourceCharacter->Controller))
		{
			// Calls the ShowDamageNumber function on the source player's controller to display floating damage text
			// above the target character. Parameters: Damage amount to display, TargetCharacter whose location will
			// be used to spawn the damage widget, bBlockedHit flag to potentially change the visual style if the
			// hit was blocked, and bCriticalHit flag to show special styling for critical hits.
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);

			// Returns early from the function after showing damage to the source player, preventing the code below
			// from executing. This is critical to avoid showing duplicate damage numbers when both the attacker and
			// victim are players (PvP scenario). Since the attacker already saw the damage they dealt, we don't need
			// to also show it to the victim.
			return;
		}

		// Attempts to cast the target character's controller to AFoxPlayerController to check if the target
		// is controlled by a human player. This second check only executes if the first check failed (due to
		// the early return above), meaning the attacker was AI-controlled. If this cast succeeds, PC will contain
		// a valid pointer to the victim's player controller, allowing us to show damage feedback to the defending
		// player. If this cast also fails, neither block executes and no damage numbers appear (AI vs AI scenario).
		if(AFoxPlayerController* PC = Cast<AFoxPlayerController>(Props.TargetCharacter->Controller))
		{
			// Calls ShowDamageNumber on the target player's controller to display floating damage text above their
			// own character (TargetCharacter). Note that even though this is the target's controller, we still
			// pass TargetCharacter (which is the victim) as the location parameter because damage numbers should
			// appear above the character who took damage, not the attacker.
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
		}
	}
}

void UFoxAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	/**
	 * Checks if the target character (the enemy that died) implements the UCombatInterface interface.
	 * 
	 * UNREAL INTERFACE PATTERN - UCombatInterface (U-prefix) vs ICombatInterface (I-prefix):
	 * 
	 * UCombatInterface: The UINTERFACE wrapper for Unreal's reflection system. Inherits from UInterface,
	 * contains no actual functionality, used only for type checking with Implements<UCombatInterface>().
	 * 
	 * ICombatInterface: The actual interface containing all virtual functions and BlueprintNativeEvents.
	 * Classes inherit from this to implement functionality. Execute_ functions are called through this
	 * class using scope resolution (ICombatInterface::Execute_FunctionName).
	 * 
	 * WHY THE DUAL PATTERN: Implements<UCombatInterface>() uses the U-prefix because Unreal's reflection
	 * system works with UObject-derived classes. However, Execute_ functions are static members of the
	 * I-prefixed class generated by UHT, requiring ICombatInterface:: scope resolution for calls.
	 * 
	 * EXAMPLE: if (Actor->Implements<UCombatInterface>()) { ICombatInterface::Execute_GetPlayerLevel(Actor); }
	 * 
	 * This runtime check ensures the target implements the interface before accessing GetPlayerLevel() and
	 * GetCharacterClass() for XP calculations, preventing crashes on non-combat actors. In this project,
	 * AFoxCharacterBase implements ICombatInterface, inherited by both AFoxCharacter (player) and
	 * AFoxEnemy (AI enemies).
	*/
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		/**
		 * Retrieves the player level of the target character through the ICombatInterface.
		 * 
		 * The Execute_ prefix is Unreal's auto-generated C++ way to call BlueprintNativeEvent functions.
		 * Execute_GetPlayerLevel checks if a Blueprint override exists, if not it calls GetPlayerLevel_Implementation.
		 * First parameter (Props.TargetCharacter) is the UObject implementing the interface (the enemy that died).
		*/
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);

		// Calls the Blueprint implementable GetCharacterClass() function using the Execute_ prefix, which is the
		// auto-generated C++ way to invoke BlueprintNativeEvent functions. This retrieves the target's character
		// class (enum value like Warrior, Ranger, or Elementalist) by passing the target character as the UObject
		// parameter, enabling class-specific XP reward calculations where different enemy types grant different
		// amounts of experience even at the same level
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);

		// Calculates the appropriate XP reward amount by calling a static utility function that looks up the XP
		// value in a data table based on both the target's character class and level.
		const int32 XPReward = UFoxAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

		// Retrieves the singleton instance of FFoxGameplayTags, which provides centralized access to all gameplay tags
		// used throughout the project. By storing this as a const reference, we avoid unnecessary copies and ensure
		// we're accessing the authoritative tag definitions. The Get() function returns the single shared instance
		// that was initialized at startup, allowing us to access tags like Attributes_Meta_IncomingXP without having
		// to define them locally or pass them as parameters
		const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

		// Declares an FGameplayEventData struct which serves as a data container (payload) for packaging information
		// that will be sent along with a gameplay event. This struct can hold various pieces of event-related data
		// including the event tag (to identify what type of event this is), magnitude (numerical value associated
		// with the event), instigator references, target data, and contextual information. We'll populate this struct
		// with XP-specific data and then send it to the ability system for processing
		FGameplayEventData Payload;

		// Sets the EventTag field of the payload to the IncomingXP meta attribute tag, which categorizes this event
		// as an experience point gain event. This tag acts as an identifier that allows gameplay abilities to listen
		// for and respond to this specific event type. When this event is sent, any ability that has registered to
		// trigger on the Attributes_Meta_IncomingXP tag will be notified and can react accordingly (such as a
		// passive ability that handles leveling up logic when XP is received)
		Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;

		// Sets the EventMagnitude field of the payload to the calculated XP reward value, specifying the actual
		// amount of experience points that should be granted. This numerical value will be accessible to any
		// gameplay abilities that respond to this event, allowing them to know exactly how much XP to award.
		// The magnitude is stored as a float to support fractional XP values if needed, though in this case
		// it's being set from an int32 XPReward which will be implicitly converted
		Payload.EventMagnitude = XPReward;

		
		// Calls the static utility function SendGameplayEventToActor to send the XP gain event to the source
		// character's ability system component. This function locates the ASC on Props.SourceCharacter (the player
		// who killed the enemy), sends an event with the Attributes_Meta_IncomingXP tag, and includes the Payload
		// containing the XP amount.
		//
		// There are TWO different ways gameplay abilities can respond to this event:
		//
		// METHOD 1 - Ability Triggers (Automatic Activation):
		//   Configure the "Ability Triggers" property in the ability's class defaults to listen for the
		//   Attributes_Meta_IncomingXP tag. When this event is sent, any ability with a matching trigger tag
		//   will AUTOMATICALLY ACTIVATE without needing to be running already. This is ideal for passive
		//   abilities that should respond to events (like leveling up when XP is gained). The ability activates,
		//   can access Payload.EventMagnitude to get the XP amount, processes the logic, and then ends.
		//
		// METHOD 2 - Wait Gameplay Event Node (Manual Listening):
		//   Use a "Wait Gameplay Event" node inside an ALREADY ACTIVE ability's blueprint graph. The ability
		//   must be activated first (either manually or through other triggers), then it reaches the Wait node
		//   and pauses execution until an event with the matching tag arrives. When the event is received, the
		//   node outputs the Payload data and continues executing the ability graph. This is useful for abilities
		//   that need to react to events as part of a larger ongoing ability sequence, or when you need more
		//   control over when the ability starts listening for events.
		//
		// In this case we use method 2 in the GA_ListenForEvent blueprint
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	}
}

// Step 3 to adding attributes
void UFoxAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	/*
	GAMEPLAYATTRIBUTE_REPNOTIFY is a GAS helper macro that handles prediction reconciliation on clients by comparing
	the replicated attribute value with the client's predicted value, updating the AbilitySystemComponent's cached 
	attribute aggregators, broadcasting OnAttributeChange delegates to notify listeners (like UI widgets), 
	and ensuring proper synchronization between server authority and client predictions - it takes the AttributeSet 
	class (UFoxAttributeSet), the attribute property name (Health), and the old value (OldHealth) to perform these 
	operations automatically.
	*/
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Health, OldHealth);
}

void UFoxAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Mana, OldMana);
}

void UFoxAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Strength, OldStrength);
}

void UFoxAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Intelligence, OldIntelligence);
}

void UFoxAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Resilience, OldResilience);
}

void UFoxAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Vigor, OldVigor);
}

void UFoxAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, Armor, OldArmor);
}

void UFoxAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UFoxAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, BlockChance, OldBlockChance);
}

void UFoxAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UFoxAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UFoxAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UFoxAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UFoxAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UFoxAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, MaxHealth, OldMaxHealth);
}

void UFoxAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, MaxMana, OldMaxMana);
}

void UFoxAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, FireResistance, OldFireResistance);
}

void UFoxAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, LightningResistance, OldLightningResistance);
}

void UFoxAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UFoxAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UFoxAttributeSet, PhysicalResistance, OldPhysicalResistance);
}

void UFoxAttributeSet::Debuff(const FEffectProperties& Props)
{
	// Retrieves the singleton instance of FFoxGameplayTags to access the DamageTypesToDebuffs map, which associates
	// damage type tags (Fire, Lightning, Arcane, Physical) with their corresponding debuff tags (Debuff.Burn,
	// Debuff.Stun, Debuff.Arcane, Debuff.Bleed) for dynamically applying the correct debuff effect based on the
	// damage type that triggered this debuff application
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	// Creates a new FGameplayEffectContext from the source ability system component to serve as the context
	// for the dynamically-created debuff gameplay effect, which will track metadata like who applied the effect,
	// when it was applied, and other contextual information needed for proper effect execution and replication.
	// The MakeEffectContext function returns a handle to the context and we store it in this variable
	FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();

	// Adds the source avatar actor (the character who dealt the damage that triggered this debuff) to the effect
	// context, which establishes proper attribution for the debuff effect and ensures that damage events from the
	// periodic debuff ticks can be traced back to the original instigator for damage feedback, XP rewards, and
	// kill credit purposes
	EffectContext.AddSourceObject(Props.SourceAvatarActor);

	// Extracts the damage type tag (Fire, Lightning, Arcane, or Physical) from the original gameplay effect context (not
	// the one we just created but the one passed in to the current function as Props.EffectContextHandle)
	// handle using the custom GetDamageType() utility function, which retrieves the damage type that was set when
	// the damaging ability was executed, allowing us to determine which type of debuff should be applied (burn for
	// fire damage, stun for lightning damage, etc.)
	const FGameplayTag DamageType = UFoxAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);

	// Extracts the debuff damage magnitude from the original gameplay effect context handle using the custom
	// GetDebuffDamage() utility function, which retrieves the damage-per-tick value of the debuff effect
	const float DebuffDamage = UFoxAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);

	// Extracts the debuff duration from the original gameplay effect context handle using the custom GetDebuffDuration()
	// utility function, which retrieves how long the debuff effect should persist on the target before expiring,
	// measured in seconds (e.g., a burn effect lasting 5 seconds)
	const float DebuffDuration = UFoxAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);

	// Extracts the debuff frequency (tick interval) from the original gameplay effect context handle using the custom
	// GetDebuffFrequency() utility function, which retrieves how often the periodic debuff damage should be applied,
	// measured in seconds (e.g., dealing damage every 1 second for a burn effect)
	const float DebuffFrequency = UFoxAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);
	
	// Constructs a unique name string for the dynamically-created debuff gameplay effect by concatenating the prefix
	// "DynamicDebuff_" with the damage type tag's string representation (e.g., "DynamicDebuff_Damage.Fire"), which
	// allows for debugging and identification of which type of debuff effect was created at runtime. The *DamageType
	// dereference operator converts the FGameplayTag to a string, and FString::Printf formats it into the final name
	FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageType.ToString());

	// Creates a new UGameplayEffect object at runtime (rather than using a pre-defined asset in the editor) by calling
	// NewObject with the transient package (temporary memory not saved to disk) and the constructed name, allowing us
	// to configure a debuff effect dynamically with custom damage, duration, and frequency values extracted from the
	// original damaging effect's context rather than creating separate static gameplay effect assets for every possible
	// damage/duration/frequency combination
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));

	// Sets the duration policy of the gameplay effect to HasDuration, which means this effect will automatically expire
	// after a specified time period rather than persisting indefinitely (Infinite) or applying instantly (Instant),
	// allowing the debuff to have a limited lifespan as defined by the DurationMagnitude value we'll set next
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// Sets the Period property of the gameplay effect to the extracted debuff frequency value, which determines how
	// often the effect executes its periodic modifiers (damage ticks) while active. For example, if DebuffFrequency
	// is 1.0, the debuff will apply its damage modifier every 1 second until the effect expires, creating the
	// damage-over-time behavior characteristic of debuffs like burning or bleeding
	Effect->Period = DebuffFrequency;

	// Sets the DurationMagnitude property of the gameplay effect to a scalable float wrapping the extracted debuff
	// duration value, which determines how long the effect persists on the target before automatically expiring.
	// FScalableFloat allows the duration to potentially scale with game difficulty or other factors if configured
	// in curve tables, though in this case we're using a fixed value from the original effect's context
	Effect->DurationMagnitude = FScalableFloat(DebuffDuration);
	
	// The following 4 lines were added to fix a deprecation warning for this line:
	// Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.DamageTypesToDebuffs[DamageType]);
	
	// Declares an FInheritedTagContainer struct to hold the tag modifications we want to apply to the target, which
	// supports adding tags (Added), removing tags (Removed), and parent tags for hierarchical inheritance. This
	// container acts as a staging area where we'll specify which tags the debuff effect should grant to its target before
	// committing the changes to the TargetTagsComponent
	FInheritedTagContainer InheritedTagContainer;
	
	// Retrieves the UTargetTagsGameplayEffectComponent from the effect (or creates one if it doesn't exist) and stores
	// a reference to it, which is the component responsible for managing gameplay tags that should be added to or
	// removed from the target actor while this effect is active. We need this component to grant the appropriate debuff
	// tag (like Debuff.Burn or Debuff.Stun) to the target so other systems can query whether the target is debuffed
	UTargetTagsGameplayEffectComponent& TargetTagsComponent = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();

	// Adds the appropriate debuff tag to the InheritedTagContainer's Added collection by looking up the damage type
	// in the DamageTypesToDebuffs map (which maps Fire->Debuff.Burn, Lightning->Debuff.Stun, etc.), ensuring that
	// when this effect is applied, the target receives the correct debuff identification tag that can be queried by
	// other gameplay systems, UI elements, or abilities that need to know the target's current debuff status
	InheritedTagContainer.Added.AddTag(GameplayTags.DamageTypesToDebuffs[DamageType]);

	// Applies the populated InheritedTagContainer to the TargetTagsComponent, which commits the tag changes we
	// configured (adding the debuff tag) to the component's internal tag grants, ensuring that when this gameplay
	// effect is applied to a target, the target's ability system component will be granted the debuff tag for the
	// duration of the effect
	TargetTagsComponent.SetAndApplyTargetTagChanges(InheritedTagContainer);
	
	// Even though the following line causes a deprecation warning We must use it for now because the setter function 
	// (Effect->SetStackingType(EGameplayEffectStackingType::AggregateBySource);) causes a linker error in this version 
	// of the engine perhaps in the future that will change. However, it does not really matter because we are only 
	// creating this gameplay effect from C++ for educational purposes this gameplay effect could be moved to blueprint.
	// Due to this we disable the deprecation warning for now
	//
	// Configures the gameplay effect's stacking behavior to AggregateBySource, which means that multiple applications
	// of this debuff from the SAME source actor will stack together (refreshing duration and potentially increasing
	// magnitude depending on configuration), while applications from DIFFERENT source actors will be treated as
	// separate effect instances. This prevents a single attacker from applying unlimited stacks while still allowing
	// multiple different attackers to each apply their own debuff stack to the same target
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// Sets the maximum number of stacks that can exist from a single source to 1, which means if the same attacker
	// applies this debuff multiple times to the same target, only one instance will be active at a time and subsequent
	// applications will refresh the duration rather than adding additional stacks. This prevents a single source from
	// stacking debuff damage to extreme levels while still allowing the debuff to be refreshed by repeated applications
	Effect->StackLimitCount = 1;

	// Calculates the index where we'll add a new modifier to the effect's Modifiers array by getting the current
	// array size, which will be the next available slot. Since array indices are zero-based and Num() returns the
	// count of existing elements, this index points to the position immediately after the last element where our
	// new modifier will be inserted
	const int32 Index = Effect->Modifiers.Num();

	// Adds a new default-constructed FGameplayModifierInfo struct to the effect's Modifiers array, expanding the array
	// by one element and returning the new size. This creates an empty modifier slot that we'll populate with our
	// debuff damage configuration (magnitude, operation type, and target attribute) in the subsequent lines. Each
	// modifier represents a single attribute modification that the gameplay effect will apply
	Effect->Modifiers.Add(FGameplayModifierInfo());

	// Retrieves a non-const reference to the FGameplayModifierInfo we just added at the calculated index position,
	// allowing us to configure the modifier's properties (magnitude, operation type, and attribute) in the following
	// lines. Using a reference avoids copying the struct and ensures we're modifying the actual modifier stored in
	// the effect's Modifiers array rather than a temporary copy
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];

	// Sets the magnitude of the modifier to the extracted debuff damage value wrapped in an FScalableFloat, which
	// determines how much damage each periodic tick of the debuff will apply to the target. FScalableFloat allows
	// the damage to potentially scale with curve tables if configured, though we're using a fixed value extracted
	// from the original effect's context that was set when the damaging ability was executed
	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);

	// Sets the modifier operation to Additive, which means the modifier's magnitude will be added to the target
	// attribute's current value rather than multiplying it (Multiplicative) or overriding it (Override).
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	// Sets the target attribute of the modifier to the IncomingDamage meta attribute using the generated static getter
	// function, which specifies that this modifier should add its magnitude value to the IncomingDamage attribute on
	// each periodic tick. IncomingDamage is a meta attribute that doesn't directly store a value but triggers damage
	// processing logic in PostGameplayEffectExecute, allowing the debuff damage to flow through the same damage
	// pipeline as direct attacks (including hit reactions, death checks, and floating text)
	ModifierInfo.Attribute = UFoxAttributeSet::GetIncomingDamageAttribute();

	// Creates a new FGameplayEffectSpec on the heap (manual memory allocation via new) that wraps the configured
	// UGameplayEffect with the provided context and level 1.0 (since we do not use a curve table for the debuff damage 
	// yet), then checks if the allocation succeeded before proceeding. The spec represents a concrete instantiation of
	// the gameplay effect ready to be applied to a target, containing all the effect's configuration plus contextual 
	// information about who applied it and when. Using a conditional check with assignment ensures we only proceed if 
	// the spec was successfully created
	if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f))
	{
		// Casts the generic FGameplayEffectContext retrieved from the spec to our custom FFoxGameplayEffectContext
		// subclass using static_cast, which gives us access to project-specific context data like damage types, debuff
		// parameters, and critical hit flags that aren't available in the base context class. The Get() call retrieves
		// the raw pointer from the shared pointer wrapper, and static_cast is safe here because we know this context
		// was created as an FFoxGameplayEffectContext (as evidenced by our earlier calls to custom setters)
		FFoxGameplayEffectContext* FoxContext = static_cast<FFoxGameplayEffectContext*>(MutableSpec->GetContext().Get());

		// Creates a TSharedPtr (Unreal's reference-counted smart pointer) wrapping a new FGameplayTag initialized with
		// the damage type tag value, which is necessary because the custom context's SetDamageType function expects a
		// shared pointer parameter rather than a raw pointer or value. MakeShareable constructs the shared pointer with
		// proper reference counting, ensuring the tag's memory will be automatically cleaned up when no longer referenced
		// MakeShared is a more efficient alternative potentially, but perhaps this could cause some other issue. We leave
		// that for further research, since this effect creation and application will probably be done in BP at some point
		TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));

		// Stores the damage type tag in the custom gameplay effect context by calling SetDamageType, which preserves
		// information about what type of damage (Fire, Lightning, Arcane, Physical) caused this debuff. This allows
		// downstream systems like damage calculations, floating text displays, and hit reactions to query the damage
		// type from the debuff's periodic damage ticks, maintaining consistent damage type attribution throughout the
		// debuff's lifetime even though the original damaging ability has finished executing
		FoxContext->SetDamageType(DebuffDamageType);

		// Applies the fully-configured gameplay effect spec to the target's ability system component, which adds the
		// debuff effect to the target's active gameplay effects, grants the debuff tag to the target for the effect's
		// duration, and begins executing periodic damage ticks according to the configured frequency and magnitude.
		// The effect will persist on the target until its duration expires or it's manually removed, applying damage
		// to the IncomingDamage attribute each tick which triggers damage processing in PostGameplayEffectExecute
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}
