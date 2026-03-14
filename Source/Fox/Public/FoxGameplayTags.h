// Copyright TryingToMakeGames

#pragma once

#include "CoreMinimal.h"
#include "AudioDeviceManager.h"
#include "GameplayTagContainer.h"

/**
 * FoxGameplayTags
 * 
 * Singleton containing native (created and available in C++ but also available in blueprints and the editor) Gameplay Tags
 */
struct FFoxGameplayTags
{
public:
	
	/* 
	* Static functions belong to the class itself instead of instances of the class.
	* This function returns the singleton instance of this class that contains the gameplay tags
	*/
	static const FFoxGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();
	
	// Convention is to use _ in place of . to identify the gameplay tags
	// Gameplay tags to identify primary attributes
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;
	
	// Gameplay tags to identify secondary attributes
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	
	// Gameplay tags to identify the IncomingXP meta attribute
	FGameplayTag Attributes_Meta_IncomingXP;

	// Gameplay tags to identify input
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	
	// Gameplay tag to identify damage
	FGameplayTag Damage;
	
	// Gameplay tags to identify damage type
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;
	
	// Gameplay tags to identify damage resistances. These are secondary attributes as well.
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;
	
	// Gameplay tags to identify debuffs
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Arcane;
	FGameplayTag Debuff_Physical;
	
	// Gameplay tags to identify debuff attributes
	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Duration;
	FGameplayTag Debuff_Frequency;
	
	// Gameplay tag to identify that no ability is assigned to a spell globe
	FGameplayTag Abilities_None;
	
	// Gameplay tag to identify the attack ability
	FGameplayTag Abilities_Attack;
	
	// Gameplay tag to identify the summon ability
	FGameplayTag Abilities_Summon;
	
	// Gameplay tag to identify the hit react ability
	FGameplayTag Abilities_HitReact;

	// Gameplay tag to identify the status of an ability
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;

	// Gameplay tag to identify the type of an ability, which determines if it should be shown in the spell menu
	// Those with type None will not be shown in the spell menu
	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	
	// Gameplay tag to identify the FireBolt ability
	FGameplayTag Abilities_Fire_FireBolt;
	
	// Gameplay tag to identify the Electrocute ability
	FGameplayTag Abilities_Lightning_Electrocute;
	
	// Gameplay tags to identify passive spell abilities
	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;
	
	// Gameplay tag to identify the FireBolt ability cooldown Gameplay Effect
	FGameplayTag Cooldown_Fire_FireBolt;
	
	// Tags to identify a characters combat sockets
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;
	
	// Gameplay tags to identify attack montages
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	
	// Map for associating damage types tags with their corresponding resistances tags
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	
	// Map for associating damage types tags with their corresponding debuff tags
	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
	
	// Gameplay tag granted when hit reacting
	FGameplayTag Effects_HitReact;
	
	// Gameplay tags for blocking game mechanics.
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	
private:
	
	/**
	 * Static Member Variable for Singleton Pattern
	 * 
	 * This static member variable holds the single instance of FFoxGameplayTags,
	 * implementing the Singleton pattern for this class.
	 * 
	 * Understanding Static Variables vs Static Functions:
	 * - Static functions (like Get() and InitializeNativeGameplayTags()) belong to the CLASS itself,
	 *   not to any particular instance, and can be called directly on the class name
	 * - Static functions do NOT require a static member variable to exist or function
	 * - Most static functions are simple utility functions that don't need any instance at all
	 * - Static functions can work with parameters, local variables, and other static members
	 * - A static variable is a variable that belongs to the class itself rather than any specific object (instance).
	 *   It is allocated in memory only once when the program starts and is shared by every instance of that class,
	 *   allowing it to maintain its value throughout the entire lifecycle of the application.
	 * 
	 * Why This Class Has a Static Variable:
	 * This particular class uses a static variable specifically to implement the Singleton pattern.
	 * The Singleton pattern ensures only ONE instance of this class exists throughout the application.
	 * - GameplayTags is the single instance, stored in static memory, created at program startup
	 * - The Get() static function provides controlled access to this single instance
	 * - This is a design choice for this class, not a requirement for static functions in general
	 * 
	 * Static Variable Declaration and Definition in C++:
	 * In C++, static member variables require both declaration AND definition:
	 * 1. Declaration (here in .h file): Tells the compiler this static member exists
	 *    Syntax: static FFoxGameplayTags GameplayTags;
	 * 
	 * 2. Definition (in .cpp file): Actually allocates memory for the variable
	 *    Syntax: FFoxGameplayTags FFoxGameplayTags::GameplayTags;
	 *    - First FFoxGameplayTags: The type of the variable
	 *    - FFoxGameplayTags::: Specifies this belongs to the FFoxGameplayTags class
	 *    - GameplayTags: The name of the static member being defined
	 * 
	 * Without the definition in the .cpp file, the linker will fail with:
	 * "undefined reference to FFoxGameplayTags::GameplayTags"
	 * 
	 * This is because the header only declares the variable exists, but doesn't reserve 
	 * actual memory for it - that's the job of the definition in the implementation file.
	 */
	static FFoxGameplayTags GameplayTags;
};
