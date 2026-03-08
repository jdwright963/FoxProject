// Copyright TryingToMakeGames


#include "AbilitySystem/FoxAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "FoxAbilityTypes.h"
#include "FoxGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Game/FoxGameModeBase.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/FoxPlayerState.h"
#include "UI/HUD/FoxHUD.h"
#include "UI/WidgetController/FoxWidgetController.h"

bool UFoxAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AFoxHUD*& OutFoxHUD)
{
	// Gets the player controller of the local player 0 since widgets are only relevant to them
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		// Gets the HUD from the player controller and casts it to FoxHUD
		OutFoxHUD = Cast<AFoxHUD>(PC->GetHUD());
		
		// Checks if the OutFoxHUD is valid
		if (OutFoxHUD)
		{
			// Get the player state using a template function that returns it in the type specified (AFoxPlayerState)
			AFoxPlayerState* PS = PC->GetPlayerState<AFoxPlayerState>();
			
			// Get the ability system component
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			
			// Get the attribute set
			UAttributeSet* AS = PS->GetAttributeSet();

			// Populate the output struct's AttributeSet member with the attribute set we retrieved
			OutWCParams.AttributeSet = AS;

			// Populate the output struct's AbilitySystemComponent member with the ASC we retrieved
			OutWCParams.AbilitySystemComponent = ASC;

			// Populate the output struct's PlayerState member with the player state we retrieved
			OutWCParams.PlayerState = PS;

			// Populate the output struct's PlayerController member with the player controller we retrieved
			OutWCParams.PlayerController = PC;

			// Return true to indicate that the widget controller params were successfully created
			return true;
		}
	}
	
	// Return false to indicate that the widget controller params were NOT successfully created
	return false;
}

UOverlayWidgetController* UFoxAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	// Create an instance of FWidgetControllerParams struct that will be populated by MakeWidgetControllerParams
	FWidgetControllerParams WCParams;

	// Declare a pointer to AFoxHUD and initialize it to nullptr. This will also be populated by MakeWidgetControllerParams
	AFoxHUD* FoxHUD = nullptr;

	// Call helper function to populate WCParams and FoxHUD. Returns true if successful, false otherwise
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, FoxHUD))
	{
		// If successful, get the overlay widget controller from FoxHUD using the populated widget controller params
		// This is a different function with the same name on a different class
		return FoxHUD->GetOverlayWidgetController(WCParams);
	}
	
	// Return nullptr if any of the if statements are false
	return nullptr;
}

UAttributeMenuWidgetController* UFoxAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	// Create an instance of FWidgetControllerParams struct that will be populated by MakeWidgetControllerParams
	FWidgetControllerParams WCParams;

	// Declare a pointer to AFoxHUD and initialize it to nullptr. This will also be populated by MakeWidgetControllerParams
	AFoxHUD* FoxHUD = nullptr;

	// Call helper function to populate WCParams and FoxHUD. Returns true if successful, false otherwise
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, FoxHUD))
	{
		// If successful, get the overlay widget controller from FoxHUD using the populated widget controller params
		// This is a different function with the same name on a different class
		return FoxHUD->GetAttributeMenuWidgetController(WCParams);
	}
	
	// Return nullptr if any of the if statements are false
	return nullptr;
}

USpellMenuWidgetController* UFoxAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	// Create an instance of FWidgetControllerParams struct that will be populated by MakeWidgetControllerParams
	FWidgetControllerParams WCParams;

	// Declare a pointer to AFoxHUD and initialize it to nullptr. This will also be populated by MakeWidgetControllerParams
	AFoxHUD* FoxHUD = nullptr;

	// Call helper function to populate WCParams and FoxHUD. Returns true if successful, false otherwise
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, FoxHUD))
	{
		// If successful, get the overlay widget controller from FoxHUD using the populated widget controller params
		// This is a different function with the same name on a different class
		return FoxHUD->GetSpellMenuWidgetController(WCParams);
	}
	
	// Return nullptr if any of the if statements are false
	return nullptr;
}

void UFoxAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	// Gets the Avatar Actor from the Ability System Component
	AActor* AvatarActor = ASC->GetAvatarActor();
	
	// Gets the game mode and then from that it gets the UCharacterClassInfo data asset
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	
	// Use its member function to get the FCharacterClassDefaultInfo struct that is mapped to the CharacterClass enum input
	// parameter
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	// Make an effect context for the primary attributes and store its handle in a variable
	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	
	// Add the Avatar Actor to the effect context as the source object
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	
	// Make a spec handle for the PrimaryAttributes effect member variable of ClassDefaultInfo, passing in the Level input 
	// parameter of the current function and the effect context we create in place.
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	
	// Apply the primary attributes effect spec to the ability system component. This function takes an effect spec not 
	// a handle. So we access the Data member of the handle to get the effect spec but this is a weak pointer. So, we 
	// use Get() to get the actual effect spec pointer and then we use '*' to dereference it (get the pointers value)
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());
	
	// Make an effect context for the secondary attributes and store its handle in a variable
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	
	// Add the Avatar Actor to the effect context as the source object
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	
	// Similar to the above just for the secondary attributes effect. However the effect here is a member variable of the 
	// UCharacterClassInfo* CharacterClassInfo data asset
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());
	
	// Make an effect context for the vital attributes and store its handle in a variable
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	
	// Add the Avatar Actor to the effect context as the source object
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	
	// Similar to the above just for the vital attributes effect.
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UFoxAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	// Calls a function that gets the game mode and then from that it gets the UCharacterClassInfo data asset
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	
	// If the data asset is null, return early
	if (CharacterClassInfo == nullptr) return;
	
	// Iterate through the CommonAbilities array member variable of CharacterClassInfo
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		// Create ability spec for the current ability class in the loop
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		
		// Give the current ability to the ASC and the Avatar Actor of that ASC
		ASC->GiveAbility(AbilitySpec);
	}
	
	// Get from the data asset the instance of FCharacterClassDefaultInfo that maps to the value of CharacterClass
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	
	// Loop through the items in the StartupAbilities array member variable of DefaultInfo
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		// Check if the Avatar Actor implements the UCombatInterface using Unreal's Implements<T>() template function.
		// This is necessary because we need to call GetPlayerLevel() from the interface, but not all actors implement it.
		// Implements<T>() returns true if the actor implements the specified interface, false otherwise. UCombatInterface
		// Is used with Implements<T>() and ICombatInterface is used to call BlueprintNativeEvent interface functions
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			// Create an ability spec for the current ability class, setting its level to match the avatar actor's player level.
			// We use ICombatInterface::Execute_GetPlayerLevel() to call the interface function on the avatar actor.
			// Execute_ is Unreal's generated function for calling BlueprintNativeEvent interface functions that handles
			// both C++ implementations and Blueprint overrides. This ensures abilities scale with the character's level.
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			
			// Give the current ability to the ASC and the Avatar Actor of that ASC
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

int32 UFoxAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	// Calls a function that gets the game mode and then from that it gets the UCharacterClassInfo data asset
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	
	// If the data asset is null, return early (we return 0 since this function must return an integer)
	if (CharacterClassInfo == nullptr) return 0;
	
	/*
	 * Get from the data asset the instance of FCharacterClassDefaultInfo that maps to the value of CharacterClass
	 * passed into this function. This struct contains class-specific information including the XPReward scalable float.
	 * We use a const reference (&) to avoid copying the struct data, improving performance while ensuring we don't
	 * modify the original data in the CharacterClassInformation TMap.
	 */
	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	/*
	 * Retrieve the XP reward value for the specified character level using FScalableFloat's GetValueAtLevel function.
	 * FScalableFloat is an Unreal structure that stores a base float value and an optional curve table. The 
	 * GetValueAtLevel function evaluates the curve table at the given level to return a scaled float value. If no 
	 * curve table is assigned, it returns the base value. This allows designers to configure XP rewards that scale 
	 * with character level (e.g., higher level enemies give more XP) without code changes.
	 */
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	
	/*
	 * Convert the float XP reward value to an int32 using static_cast for the return value. static_cast is a C++ 
	 * compile-time cast that truncates the decimal portion (e.g., 150.75 becomes 150). 
	 * The function returns int32 rather than float because experience points are typically whole numbers in game 
	 * systems, and int32 provides sufficient range (±2.1 billion) for XP values while being more memory efficient 
	 * than int64.
	 * 
	 * Why static_cast vs Unreal's Cast macro:
	 * - Unreal's Cast<T>() macro is specifically designed for casting between UObject-derived types (classes)
	 * - Cast<T>() performs runtime type safety checks using RTTI and Unreal's reflection system
	 * - static_cast is for fundamental type conversions (float to int32) and compile-time known pointer casts
	 * - Using Cast<T>() for fundamental types would be incorrect - it's not designed for that purpose
	 * - static_cast for fundamental types has no runtime overhead, while Cast<T>() has runtime type checking cost
	 * 
	 * Why static_cast vs C-style cast (e.g., (int32)XPReward):
	 * - C-style casts are inherited from C and work in both C and C++, using syntax like (TargetType)value
	 * - static_cast is a modern C++ casting operator (introduced in C++98) that provides compile-time type checking
	 * - static_cast is more explicit about the programmer's intent, making code more readable and maintainable
	 * - static_cast is searchable in code (can find "static_cast" but harder to search for all C-style casts)
	 * - static_cast will produce compiler errors if the conversion is not allowed, catching bugs earlier
	 * - C-style casts can perform dangerous conversions (like casting away const) without warning
	 */
	return static_cast<int32>(XPReward);
}

UCharacterClassInfo* UFoxAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// Get the game mode and casts it to AFoxGameModeBase
	AFoxGameModeBase* AuraGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	
	// Return early if the game mode is null
	if (AuraGameMode == nullptr) return nullptr;
	
	// Now that we have the game mode we have access to the UCharacterClassInfo data asset stored on it. We access it and
	// store it in a variable here
	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UFoxAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	
	// Get the game mode from the world context object and cast it to AFoxGameModeBase. This function uses the const
	// version of Cast since we only need to read from the game mode (access its AbilityInfo data asset) and don't
	// need to modify it.
	const AFoxGameModeBase* FoxGameMode = Cast<AFoxGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));

	// Return nullptr early if the cast failed (game mode doesn't exist or isn't of type AFoxGameModeBase).
	if (FoxGameMode == nullptr) return nullptr;

	// Now that we have a valid game mode, access and return the UAbilityInfo data asset stored on it. This data
	// asset contains configuration information for all abilities in the game including their tags, icons, materials,
	// level requirements, and ability classes. This function is typically called by systems that need to look up
	// ability display information or configuration data.
	return FoxGameMode->AbilityInfo;
}

bool UFoxAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the IsBlockedHit() member function on our custom context to get the 
		// blocked hit boolean value and return it.
		return FoxEffectContext->IsBlockedHit();
	}
	// If the cast failed (returned nullptr), return false as the default value indicating the hit was not blocked.
	return false;
}

bool UFoxAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the IsSuccessfulDebuff() member function on our custom context to get
		// the successful debuff boolean value and return it. This boolean indicates whether a debuff effect 
		// (such as Burn, Stun, Slow, etc.) was successfully applied to the target during damage calculation. 
		return FoxEffectContext->IsSuccessfulDebuff();
	}
	// If the cast failed (returned nullptr), return false as the default value indicating the debuff was not successful.
	return false;
}

float UFoxAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the GetDebuffDamage() member function on our custom context to get
		// the debuff damage float value and return it. This value indicates the damage that the debuff causes.
		return FoxEffectContext->GetDebuffDamage();
	}
	// If the cast failed (returned nullptr), return 0.f as the default value indicating no debuff damage.
	return 0.f;
}

float UFoxAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the GetDebuffDuration() member function on our custom context to get
		// the debuff duration float value and return it. This value represents the total time in seconds that a
		// debuff effect (such as Burn, Stun, Slow, etc.) will remain active on the target after being successfully
		// applied. For example, a duration of 5.0 seconds means the debuff will persist for 5 seconds before
		// expiring. This duration, combined with debuff frequency, determines how many times periodic damage ticks
		// or other debuff effects will be applied to the target.
		return FoxEffectContext->GetDebuffDuration();
	}
	// If the cast failed (returned nullptr), return 0.f as the default value indicating no debuff duration exists.
	return 0.f;
}

float UFoxAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the GetDebuffFrequency() member function on our custom context to get
		// the debuff frequency float value and return it. This value represents the time interval in seconds between
		// each application of periodic debuff damage. For example, a frequency of 1.0 means the debuff applies
		// damage every second, while 0.5 would apply damage twice per second. This frequency, combined with debuff
		// duration, determines how many damage ticks occur during the debuff's lifetime.
		return FoxEffectContext->GetDebuffFrequency();
	}
	// If the cast failed (returned nullptr), return 0.f as the default value indicating no debuff frequency exists.
	return 0.f;
}

FGameplayTag UFoxAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Check if the TSharedPtr<FGameplayTag> returned by GetDamageType() is valid (not null) before attempting to
		// dereference it. The IsValid() function is a TSharedPtr member that returns true if the pointer is not null.
		// This prevents a crash that would occur if we tried to dereference a null pointer in the return statement below.
		if (FoxEffectContext->GetDamageType().IsValid())
		{
			// Dereference the TSharedPtr<FGameplayTag> using the '*' operator to get the actual FGameplayTag value and
			// return it. GetDamageType() returns a TSharedPtr, so we must dereference it to access the underlying tag.
			// This tag represents the type of damage dealt (Fire, Lightning, Physical, etc.).
			return *FoxEffectContext->GetDamageType();
		}
	}
	// Return an empty/default-constructed FGameplayTag if either the cast to FFoxGameplayEffectContext failed (returned
	// nullptr) or if the DamageType pointer was invalid (null). An empty FGameplayTag can be checked with IsValid()
	// by calling code to determine if a valid damage type was retrieved, allowing graceful error handling.
	return FGameplayTag();
}

FVector UFoxAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the GetDeathImpulse() member function on our custom context to retrieve
		// the death impulse vector and return it. This FVector contains both the direction and magnitude of the
		// physics impulse that should be applied to a character's ragdoll when they die, creating a realistic death
		// animation that launches or pushes the body based on the killing blow's force and direction.
		return FoxEffectContext->GetDeathImpulse();
	}
	// If the cast failed (returned nullptr), return FVector::ZeroVector as the default value, indicating no death
	// impulse should be applied. ZeroVector is Unreal's static constant representing (0,0,0), meaning the character's
	// ragdoll will simply collapse in place without any additional force when they die.
	return FVector::ZeroVector;
}

FVector UFoxAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// If the cast was successful, call the GetKnockbackForce() member function on our custom context to retrieve
		// the knockback force vector and return it. This vector defines both the direction and magnitude of the physical 
		// force that displaces a living character when they take damage (unlike DeathImpulse which only applies on death).
		return FoxEffectContext->GetKnockbackForce();
	}
	// If the cast failed (returned nullptr), return FVector::ZeroVector as the default value, indicating no knockback
	// force should be applied. ZeroVector is Unreal's static constant representing (0,0,0), no knockback force will be 
	// applied
	return FVector::ZeroVector;
}

bool UFoxAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (const FFoxGameplayEffectContext* FoxEffectContext = static_cast<const FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Returns the value of the IsCriticalHit boolean indicating if a critical hit occurred.
		return FoxEffectContext->IsCriticalHit();
	}
	// If the cast failed (returned nullptr), return false as the default value indicating no critical hit occurred.
	return false;
}

void UFoxAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the value of the IsBlockedHit boolean using the setter function. This requires that the EffectContextHandle
		// input parameter be non const and not be cast to const. We also have an additional input parameter that gets 
		// passed to the setter function.
		FoxEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UFoxAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the value of the IsCriticalHit boolean using the setter function. This requires that the EffectContextHandle
		// input parameter be non const and not be cast to const. We also have an additional input parameter that gets passed
		// to the setter function.
		FoxEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UFoxAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the value of the IsSuccessfulDebuff boolean using the setter function. This requires that the
		// EffectContextHandle input parameter be non const and not be cast to const. The bInSuccessfulDebuff parameter
		// (passed into this function) is forwarded to the setter to specify whether the debuff was successfully applied.
		FoxEffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void UFoxAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the debuff damage value using the setter function. This requires that the EffectContextHandle
		// input parameter be non const and not be cast to const. The InDamage parameter (passed into this function)
		// is forwarded to the setter to specify the amount of damage that the debuff will deal per tick when applied
		// to a target. This damage value will be used by periodic gameplay effects to apply damage at the frequency
		// specified by SetDebuffFrequency over the duration specified by SetDebuffDuration.
		FoxEffectContext->SetDebuffDamage(InDamage);
	}
}

void UFoxAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the debuff duration value using the setter function. This requires that the EffectContextHandle
		// input parameter be non const and not be cast to const. The InDuration parameter (passed into this function)
		// is forwarded to the setter to specify the total time in seconds that the debuff effect will remain active
		// on the target. For example, an InDuration of 5.0 means the debuff will persist for 5 seconds before expiring.
		// This duration value, combined with the debuff frequency (time between ticks), determines how many times the
		// debuff will apply its periodic damage to the target during its lifetime.
		FoxEffectContext->SetDebuffDuration(InDuration);
	}
}

void UFoxAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Sets the debuff frequency value using the setter function. This requires that the EffectContextHandle
		// input parameter be non const and not be cast to const. The InFrequency parameter (passed into this function)
		// is forwarded to the setter to specify the time interval in seconds between each application of periodic
		// debuff damage. For example, an InFrequency of 1.0 means the debuff applies damage every second, while 0.5
		// would apply damage twice per second. This frequency value, combined with the debuff duration (total lifetime),
		// determines how many damage ticks will occur during the debuff's active period.
		FoxEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void UFoxAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		/*
		 * Create a TSharedPtr<FGameplayTag> using MakeShared<FGameplayTag>() and initialize it with the InDamageType
		 * parameter passed into this function. MakeShared is a templated function that allocates a new FGameplayTag
		 * on the heap and wraps it in a TSharedPtr for automatic reference-counted memory management. We use a shared
		 * pointer because the damage type tag needs to persist beyond this function's scope and may be accessed by
		 * multiple systems (damage calculations, gameplay cues, UI, etc.) throughout the damage effect's lifetime.
		 * TSharedPtr ensures the tag is automatically deleted when the last reference to it is released, preventing
		 * memory leaks. The const qualifier indicates this pointer itself won't be reassigned after creation, though
		 * other systems can still create their own shared references to the same underlying FGameplayTag data.
		 */
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);

		/*
		 * Store the damage type shared pointer in the Fox effect context using the SetDamageType() setter function.
		 * This makes the damage type tag (Fire, Lightning, Physical, etc.) available throughout the entire damage
		 * pipeline by storing it in the effect context. Systems can later retrieve this tag using
		 * GetDamageType() to determine how to process the damage (checking resistances, applying appropriate visual
		 * effects, playing correct sound effects, etc.). The shared pointer is passed by value to SetDamageType,
		 * which increments its reference count, ensuring the tag remains valid even after this function returns and
		 * the local DamageType variable goes out of scope.
		 */
		FoxEffectContext->SetDamageType(DamageType);
	}
}

void UFoxAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Store the death impulse vector (force and direction) in the Fox effect context using the setter function.
		// This impulse will be applied to the target's physics body when they die to create a ragdoll effect that
		// realistically launches or pushes the body in the direction of the killing blow. For example, a powerful
		// melee attack might have a large impulse that sends enemies flying backward, while a projectile might apply
		// a directional impulse along the projectile's travel path. The InImpulse parameter (passed into this function)
		// contains both the direction (normalized vector) and magnitude (vector length) of the force to apply.
		FoxEffectContext->SetDeathImpulse(InImpulse);
	}
}

void UFoxAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InForce)
{
	/*
	 * Get the raw pointer to the base FGameplayEffectContext from the handle using Get(), then use static_cast to 
	 * convert it to our custom FFoxGameplayEffectContext pointer type. 
	 *
	 * We use static_cast instead of Unreal's Cast macro because static_cast is a C++ compile-time cast that performs 
	 * no runtime type checking, making it faster but requiring the programmer to guarantee the type is correct. 
	 * Unreal's Cast macro performs runtime type safety checks using RTTI (Run-Time Type Information), which is 
	 * safer but has performance overhead. 
	 *
	 * In this case, we know with certainty that our project is configured to use FFoxGameplayEffectContext as 
	 * the effect context type (set in the project settings), so the compile-time cast is safe and more performant. 
	 * The result is stored in a non-const pointer variable called FoxEffectContext. This if statement checks if the 
	 * cast was successful (pointer is not null).
	 */
	if (FFoxGameplayEffectContext* FoxEffectContext = static_cast<FFoxGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		// Store the knockback force vector (the InForce parameter passed into this function) in the Fox effect context
		// using the setter function. This vector defines both the direction and magnitude of the physical force that 
		// displaces a living character when they take damage (unlike DeathImpulse which only applies on death).
		FoxEffectContext->SetKnockbackForce(InForce);
	}
}

void UFoxAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	// Create an instance of the FCollisionQueryParams struct using the default constructor
	FCollisionQueryParams SphereParams;
	
	// Add to this structs array of actors to ignore the array of actors to ignore passed into this function
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	// Checks if the UWorld can be retrieved from the context object passed into this function
	// `EGetWorldErrorMode::LogAndReturnNull` simply specifies the failure behavior when the world cannot be retrieved
	// it likely logs an error and returns null if the world cannot be retrieved
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		// An array of FOverlapResults that will be populated by the OverlapMultiByObjectType function with all the
		// actors that overlap with the sphere
		TArray<FOverlapResult> Overlaps;
		
		// Performs a sphere overlap query in the world to find all dynamic objects within the specified radius
		// Parameters: Overlaps array to populate, SphereOrigin as center point, FQuat::Identity for no rotation,
		// FCollisionObjectQueryParams set to check all dynamic objects, FCollisionShape::MakeSphere creates a sphere
		// of the specified Radius, and SphereParams contains actors to ignore during the query
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);

		// Iterate through each overlap result found by the sphere query to filter and collect valid actors
		for (FOverlapResult& Overlap : Overlaps)
		{
			// Check if the overlapped actor implements the UCombatInterface (supports combat functionality) AND
			// the actor is not dead by calling the IsDead interface function. Only living combat-capable actors pass
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				// Retrieve the avatar actor from the combat interface (which may differ from the component owner)
				// and add it to the output array using AddUnique to prevent duplicate entries
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

void UFoxAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors,
	TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	/*
	 * Check if the number of actors in the input array is less than or equal to MaxTargets. If true, we don't
	 * need to perform any distance calculations or sorting because all actors should be included in the result.
	 * This is an optimization that avoids unnecessary computation when the input set is already small enough.
	 * Num() is a TArray member function that returns the number of elements currently stored in the array.
	 */
	if (Actors.Num() <= MaxTargets)
	{
		/*
		 * Assign the entire Actors array to the output parameter OutClosestTargets. Since the input array
		 * contains MaxTargets or fewer actors, all of them qualify as "closest targets" by definition.
		 * 
		 * Note: TArray's assignment operator performs a "deep copy." This means it allocates a completely 
		 * new block of memory for OutClosestTargets and copies every element from the source array into it. 
		 * Unlike a "shallow copy", which would only copy a reference or pointer to the original memory, a 
		 * deep copy ensures that OutClosestTargets and Actors are entirely independent. 
		 * 
		 * This ensures the caller receives their own unique set of targets and any subsequent modifications 
		 * they make to their array (like sorting or clearing it) will not affect the original Actors array.
		 */
		OutClosestTargets = Actors;
		
		// Return early from the function to avoid executing the distance calculation and sorting logic below.
		return;
	}

	/*
	 * Create a working copy of the input Actors array that we can safely modify during the target selection process.
	 * We need a copy because the algorithm below will remove actors from this array as they are identified as closest
	 * targets (to avoid selecting the same actor multiple times), but we must not modify the original Actors array
	 * that was passed as a const reference parameter. TArray's assignment operator performs a deep copy, creating
	 * an independent array that contains the same actor pointers as the original. Modifying this copy (adding/removing
	 * elements) won't affect the caller's original array, maintaining proper const-correctness.
	 */
	TArray<AActor*> ActorsToCheck = Actors;
	
	/*
	 * Initialize a counter variable to track how many closest targets have been found so far. This starts at 0 and
	 * will be incremented each time we identify and add a closest actor to the OutClosestTargets array. The while
	 * loop below will continue executing until NumTargetsFound reaches MaxTargets, ensuring we find exactly the
	 * requested number of closest actors (or fewer if ActorsToCheck runs out of actors before reaching MaxTargets).
	 * Using int32 (Unreal's typedef for a 32-bit signed integer) ensures consistency with MaxTargets' type and
	 * provides sufficient range for target counts in gameplay scenarios.
	 */
	int32 NumTargetsFound = 0;
	
	/*
	 * Execute a while loop that continues iterating until we have found and collected exactly MaxTargets number of
	 * closest actors. The loop condition checks NumTargetsFound (initialized to 0) against MaxTargets on each
	 * iteration. Each time we identify a closest actor and add it to OutClosestTargets, NumTargetsFound is
	 * incremented by 1. The loop terminates when NumTargetsFound reaches MaxTargets, or when ActorsToCheck becomes
	 * empty (handled by the break statement inside), whichever comes first.
	 */
	while (NumTargetsFound < MaxTargets)
	{
		/*
		 * Check if the ActorsToCheck array is empty (Num() returns 0) before attempting to find the closest actor.
		 * If no actors remain in the array, immediately break out of the while loop to prevent errors. This safety
		 * check is necessary because we may exhaust all available actors before reaching MaxTargets (e.g., if
		 * MaxTargets is 10 but only 7 actors were passed to the function).
		 */
		if (ActorsToCheck.Num() == 0) break;

		/*
		 * Initialize ClosestDistance to the maximum possible value for a double-precision floating point number
		 * using TNumericLimits<double>::Max(). This ensures that the first actor we check in the loop below will
		 * always have a shorter distance than ClosestDistance, guaranteeing that ClosestActor gets assigned a valid
		 * value during the first iteration. TNumericLimits is Unreal's templated utility class that provides
		 * compile-time constants for numeric type limits (similar to std::numeric_limits in standard C++). Using
		 * Max() instead of a hardcoded large number makes the code more maintainable and eliminates potential bugs
		 * from incorrect limit values. We use double instead of float for higher precision in distance calculations,
		 * which matters when actors are very close together or very far from the origin.
		 */
		double ClosestDistance = TNumericLimits<double>::Max();

		/*
		 * Declare a pointer variable ClosestActor that will store the actor found to be nearest to the Origin point
		 * during this iteration of the while loop. This pointer is uninitialized here because it will be assigned
		 * a value in the for loop below when we find the actor with the minimum distance. After the for loop
		 * completes, ClosestActor will point to the single actor from ActorsToCheck that has the shortest distance
		 * to Origin, which we will then remove from ActorsToCheck and add to OutClosestTargets. Using AActor* (raw
		 * pointer) is appropriate here because we're referencing actors that already exist in the ActorsToCheck
		 * array and don't need ownership semantics or garbage collection management for this temporary reference.
		 */
		AActor* ClosestActor;

		/*
		 * Iterate through every actor remaining in the ActorsToCheck array using a range-based for loop to find
		 * which actor is closest to the Origin point. On each iteration, PotentialTarget is assigned a pointer to
		 * the current actor from the array, allowing us to access that actor's location and calculate its distance
		 * from Origin. This loop examines all remaining candidates (those not yet selected as closest targets) and
		 * performs a linear search to identify the single actor with the minimum distance. After this loop completes,
		 * ClosestActor will contain the nearest actor, which we can then remove from ActorsToCheck and add to the
		 * final results. The loop variable is declared as AActor* (pointer) because TArray<AActor*> stores pointers,
		 * not actor objects themselves.
		 */
		for (AActor* PotentialTarget : ActorsToCheck)
		{
			/*
			 * Calculate the distance from the Origin point to the current PotentialTarget actor's world
			 * location. First, PotentialTarget->GetActorLocation() retrieves the actor's position as an FVector.
			 * Subtracting Origin from this position creates a vector pointing from Origin to the actor.
			 * Calling Length() on this vector returns the magnitude (distance) as a double-precision
			 * floating point number. For example, if Origin is (0,0,0) and the actor is at (3,4,0), the displacement
			 * vector would be (3,4,0) and Length() would return 5.0 (using the Pythagorean theorem: sqrt(3²+4²+0²)).
			 * This distance value is stored as const because we only need to read it for comparison, not modify it.
			 */
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			
			/*
			 * Check if the current actor's distance is less than the closest distance found so far in this iteration
			 * of the while loop. If true, we've found a new closest actor and need to update both ClosestDistance
			 * and ClosestActor to reflect this new minimum. On the first iteration of the for loop, ClosestDistance
			 * is initialized to TNumericLimits<double>::Max() (the largest possible double value), guaranteeing this
			 * condition will be true for the first actor checked, ensuring ClosestActor gets assigned a valid value.
			 * In subsequent iterations, only actors closer than the current ClosestDistance will pass this check,
			 * implementing a linear search algorithm that finds the single actor with minimum distance to Origin.
			 */
			if (Distance < ClosestDistance)
			{
				/*
				 * Update ClosestDistance to store the current actor's distance value, replacing the previous closest
				 * distance. This establishes a new minimum distance benchmark that all remaining actors in the
				 * ActorsToCheck array will be compared against in subsequent iterations of the for loop. By continuously
				 * updating this value whenever we find a closer actor, we progressively narrow down to the true minimum
				 * distance by the time the for loop completes. This value will be used in the next iteration's
				 * comparison (if Distance < ClosestDistance) to determine if we've found an even closer actor.
				 */
				ClosestDistance = Distance;

				/*
				 * Update ClosestActor to point to the current PotentialTarget actor, replacing the previous closest actor
				 * reference. This assignment occurs only when we've confirmed the current actor is closer than all
				 * previously examined actors (by passing the if Distance < ClosestDistance check). After the for loop
				 * completes iterating through all actors in ActorsToCheck, ClosestActor will contain a pointer to the
				 * single actor that has the absolute minimum distance to Origin. This actor will then be removed from
				 * ActorsToCheck and added to OutClosestTargets in the while loop's cleanup code below the for loop.
				 */
				ClosestActor = PotentialTarget;
			}
		}

		/*
		 * Remove the actor identified as closest (stored in ClosestActor) from the ActorsToCheck array using the
		 * Remove() function. This deletion is critical to prevent the same actor from being selected as "closest"
		 * in subsequent iterations of the while loop, which would otherwise result in duplicate entries in
		 * OutClosestTargets. Remove() performs a linear search through the array to find the first element matching
		 * ClosestActor (using pointer equality comparison) and removes it, shifting subsequent elements down to fill
		 * the gap. After removal, the next iteration of the while loop will only consider the remaining actors when
		 * searching for the next closest target, ensuring we build a list of unique actors ordered by their distance
		 * from Origin (nearest to farthest). This is safe to call even if ActorsToCheck becomes empty afterward,
		 * because the while loop will catch that condition at the start of the next iteration with the if (ActorsToCheck.Num() == 0) check.
		 */
		ActorsToCheck.Remove(ClosestActor);

		/*
		 * Add the actor identified as closest (stored in ClosestActor) to the output array OutClosestTargets using
		 * AddUnique(). AddUnique() is a TArray member function that only adds the element if it doesn't already
		 * exist in the array (using pointer equality comparison), providing built-in duplicate prevention. Although
		 * our algorithm design (removing from ActorsToCheck immediately after finding) should already prevent
		 * duplicates, using AddUnique() adds a safety layer in case of unexpected edge cases or future code changes.
		 * This grows the OutClosestTargets array by one element per iteration, progressively building the final
		 * result of closest actors ordered by distance from Origin. By the time the while loop completes,
		 * OutClosestTargets will contain up to MaxTargets actors, sorted from nearest to farthest distance.
		 */
		OutClosestTargets.AddUnique(ClosestActor);

		/*
		 * Increment the NumTargetsFound counter by 1 using the pre-increment operator (++). This tracks how many
		 * closest actors we have successfully found and added to OutClosestTargets during this function execution.
		 * The pre-increment operator is used here (++NumTargetsFound instead of NumTargetsFound++) as a stylistic
		 * choice common in game development, though both would produce identical results since we don't use the
		 * return value. When NumTargetsFound reaches MaxTargets, the while loop condition (NumTargetsFound < MaxTargets)
		 * will evaluate to false on the next iteration check, terminating the loop and completing the function.
		 * This counter serves as the primary loop control mechanism, ensuring we collect exactly MaxTargets closest
		 * actors (or fewer if we run out of actors in ActorsToCheck first, handled by the break statement).
		 */
		++NumTargetsFound;
	}
}

bool UFoxAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	// True if both actors have the Player tag, indicating they are friends (same team)
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	
	// True if both actors have the Enemy tag, indicating they are friends (same team)
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	
	// True if both actors are either players or both actors are enemies, indicating they are friends (same team)
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	
	// Return true if the actors are not friends (different teams)
	return !bFriends;
}

FGameplayEffectContextHandle UFoxAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	/*
	 * Get a const reference to the singleton instance of FFoxGameplayTags, which contains all the gameplay tags
	 * used throughout the project. This is accessed via the static Get() function which returns a reference to
	 * the single instance created at startup. We store it in a const reference to avoid copying the struct and
	 * to make it clear we're only reading tag values. The GameplayTags variable will be used later in this
	 * function to access debuff-related tags (Debuff_Chance, Debuff_Damage, etc.) when assigning SetByCaller
	 * magnitudes to the damage effect spec.
	 */
	const FFoxGameplayTags& GameplayTags = FFoxGameplayTags::Get();

	/*
	 * Retrieve the avatar actor (the physical actor representation in the world) from the source ability system
	 * component. The source ASC is passed in via DamageEffectParams and represents the entity dealing the damage
	 * (the attacker/instigator). We need this actor reference to add it to the gameplay effect context as the
	 * source object, which allows damage calculations and other systems to query information about the attacker
	 * such as their location, team affiliation, or attributes. Stored as a const pointer since we only need to
	 * read from it, not modify it.
	 */
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	/*
	 * Create a new gameplay effect context handle using the source ability system component's MakeEffectContext()
	 * function. The context handle is a wrapper around FGameplayEffectContext which stores metadata about how and
	 * why a gameplay effect was applied, including information like the instigator, causer, hit result, ability
	 * instance, and custom data. This context will be attached to the damage effect spec and passed through the
	 * entire damage pipeline, allowing systems to access this information during damage calculations, attribute
	 * modifications, and effect application. Initially the context is empty and will be populated in the next steps.
	 */
	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();

	/*
	 * Add the source avatar actor to the effect context as a source object. This establishes the actor dealing
	 * the damage as the originator of the effect in the context's metadata. By storing the source actor in the
	 * context, we make it available throughout the entire damage pipeline. Thus, Execution calculations can query the
	 * source's attributes (like Strength for physical damage scaling), attribute sets can access the source for
	 * calculations, and gameplay cues can reference it for visual effects (spawn particles at attacker location).
	 * This is critical for systems that need to know WHO is dealing damage, not just that damage is being dealt.
	 */
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	
	SetDeathImpulse(EffectContexthandle, DamageEffectParams.DeathImpulse);
	
	SetKnockbackForce(EffectContexthandle, DamageEffectParams.KnockbackForce);

	/*
	 * Create an outgoing gameplay effect spec handle from the source ability system component using the damage
	 * effect class, ability level, and effect context we prepared. MakeOutgoingSpec() instantiates a spec from
	 * the class (DamageGameplayEffectClass), sets its level to the ability level for proper magnitude calculations,
	 * and attaches the effect context we created. The resulting spec handle points to a complete, ready-to-modify
	 * gameplay effect specification that can have SetByCaller magnitudes assigned to it before being applied to
	 * targets. This spec will be populated with damage values and debuff parameters in the following lines before
	 * being applied to the target actor's ability system component.
	 */
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);
	
	/*
	 * Assign the base damage value to the gameplay effect spec using the SetByCaller system. The DamageType tag
	 * (e.g., Damage.Fire, Damage.Lightning) identifies which damage type magnitude to set, and BaseDamage provides
	 * the numerical value. The gameplay effect class must be configured to read this tag via GetSetByCallerMagnitude()
	 * in a modifier or execution calculation to apply the damage to the target's attributes. This approach allows
	 * different damage abilities to use the same effect class while specifying different damage types and values
	 * at runtime without requiring separate effect classes for each damage type.
	 */
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);

	/*
	 * Assign the debuff application chance to the gameplay effect spec using the Debuff.Chance tag. This value
	 * represents the probability (as a percentage, e.g., 20.0 = 20% chance) that the debuff will be successfully
	 * applied to the target when this damage effect is processed. The gameplay effect's execution calculation or
	 * conditional gameplay effect application logic reads this value to perform a random roll and determine whether
	 * to apply the associated debuff effect (Burn, Stun, etc.) alongside the damage.
	 */
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);

	/*
	 * Assign the debuff damage per tick to the gameplay effect spec using the Debuff.Damage tag. This value
	 * specifies how much damage is dealt to the target each time the debuff effect ticks (applies periodic damage).
	 * The gameplay effect must be configured with a periodic execution or modifier that reads this SetByCaller
	 * magnitude and applies it to the target's health or relevant attribute. Combined with Debuff.Frequency, this
	 * determines the damage rate (e.g., 5 damage every 1 second), and combined with Debuff.Duration, it determines
	 * the total damage dealt over the debuff's lifetime.
	 */
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);

	/*
	 * Assign the debuff duration to the gameplay effect spec using the Debuff.Duration tag. This value specifies
	 * how long (in seconds) the debuff effect will persist on the target after it is successfully applied. The
	 * gameplay effect class must be configured to read this SetByCaller magnitude to set its duration policy,
	 * or the value may be used by the execution calculation to apply a timed gameplay effect. For example, a
	 * duration of 5.0 seconds with a frequency of 1.0 second means the debuff will tick 5 times before expiring.
	 */
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);

	/*
	 * Assign the debuff tick frequency to the gameplay effect spec using the Debuff.Frequency tag. This value
	 * specifies the time interval (in seconds) between each application of the debuff's periodic damage. The
	 * gameplay effect must be configured with a periodic execution policy that reads this SetByCaller magnitude
	 * to determine its period. For example, a frequency of 1.0 means the debuff damage is applied every second,
	 * while 0.5 would apply damage twice per second. This value directly controls how often Debuff.Damage is
	 * dealt to the target during the Debuff.Duration.
	 */
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	
	/*
	 * Apply the fully configured damage effect spec to the target actor's ability system component by calling
	 * ApplyGameplayEffectSpecToSelf(). This function takes a reference to an FGameplayEffectSpec (not a handle),
	 * so we must dereference the spec from the handle: SpecHandle.Data is a TSharedPtr<FGameplayEffectSpec>, calling
	 * Get() on it returns the raw FGameplayEffectSpec pointer, and using the dereference operator '*' converts that
	 * pointer to a reference (a trick presented in the tutorial is we can actually skip the Get() and the dereference 
	 * operator alone will actually return the raw pointer within TSharedPtr<FGameplayEffectSpec> and dereference it at
	 * the same time, turning that pointer into a reference). We could use `*SpecHandle.Data.Get()` as described here for
	 * the same outcome.
	 * 
	 * ApplyGameplayEffectSpecToSelf processes the spec by evaluating all modifiers and
	 * execution calculations (including our damage calculation and debuff logic), applying attribute changes to the
	 * target (reducing health, applying debuffs, etc.), and triggering any associated gameplay cues for visual/audio
	 * feedback. The "ToSelf" naming indicates the effect is applied to the ASC's own owner (the target in this case),
	 * as opposed to ApplyGameplayEffectSpecToTarget which would apply it to a different actor's ASC.
	 */
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	/*
	 * Return the effect context handle to the calling code. While the damage effect has already been applied
	 * above, returning the context handle allows the caller to access important metadata about the damage
	 * application such as: whether it was a critical hit or blocked hit (via our custom FFoxGameplayEffectContext
	 * properties), the source actor and instigator information for logging or additional processing, hit result
	 * data for spawning impact effects at the correct location, and ability instance information for chaining
	 * additional effects. For example, a projectile ability might use the returned context to determine whether
	 * to spawn a special critical hit particle effect, or a combat log system might query the context to display
	 * detailed damage information to the player. The context handle is a lightweight wrapper (just contains a
	 * shared pointer) so returning it by value is efficient and safe.
	 */
	return EffectContexthandle;
}

TArray<FRotator> UFoxAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumRotators)
{
	/*
	 * Declare an empty TArray of FRotator to store the resulting rotators that will be evenly spaced across
	 * the specified spread angle. This array will be populated in the following logic and returned to the caller.
	 * TArray is Unreal's dynamic array container that can grow/shrink as needed, making it ideal for storing
	 * a variable number of rotators based on the NumRotators input parameter. FRotator is Unreal's structure
	 * for representing 3D rotation using three angles: Pitch (rotation around Y axis), Yaw (rotation around Z
	 * axis), and Roll (rotation around X axis), measured in degrees. Rotators are the standard way to define
	 * orientation for actors, components, and projectiles in Unreal Engine.
	 */
	TArray<FRotator> Rotators;

	/*
	 * Calculate the leftmost edge of the spread angle by rotating the Forward vector by negative half the spread
	 * angle around the specified Axis. RotateAngleAxis returns a new FVector rotated by the given angle (in degrees)
	 * around the axis vector. By using -Spread / 2.f, we position this vector at the far left of the spread range.
	 * For example, if Spread is 90 degrees, this creates a vector rotated -45 degrees from Forward, establishing
	 * the starting point for distributing rotators across the spread. This leftmost position will be used as the
	 * base for incrementally rotating to create evenly spaced rotators in the subsequent loop.
	 */
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

	/*
	 * Check if more than one rotator is requested. If NumRotators is 1 or less, we don't need spacing calculations
	 * and can simply return the Forward vector's rotation. This conditional branching is necessary because the
	 * spacing calculation formula (Spread / (NumRotators - 1)) would result in division by zero if NumRotators is 1.
	 * When multiple rotators are needed, we proceed with the evenly-spaced distribution logic. Otherwise, we skip
	 * to the else block which handles the single rotator case by directly adding Forward's rotation to the array.
	 */
	if (NumRotators > 1)
	{
		/*
		 * Calculate the angular increment (in degrees) between each consecutive rotator by dividing the total spread
		 * angle by the number of gaps between rotators. The number of gaps is always one less than the number of
		 * rotators (NumRotators - 1). For example, if Spread is 90 degrees and NumRotators is 5, DeltaSpread will
		 * be 90 / (5-1) = 22.5 degrees, meaning each rotator will be spaced 22.5 degrees apart. This ensures the
		 * rotators are evenly distributed across the entire spread range from the leftmost to rightmost edge.
		 */
		const float DeltaSpread = Spread / (NumRotators - 1);

		/*
		 * Iterate through each rotator index from 0 to NumRotators - 1 (due to 0 based indexing the last element in the 
		 * array is located at the NumRotators - 1 index) to generate evenly spaced rotators across the
		 * spread angle.
		 */
		for (int32 i = 0; i < NumRotators; i++)
		{
			/*
			 * Calculate a new direction vector by rotating the leftmost spread position (LeftOfSpread) by an
			 * incrementing angle around the specified Axis. The rotation angle is DeltaSpread multiplied by the
			 * current loop index i, which creates evenly spaced rotations across the spread range. When i=0,
			 * Direction equals LeftOfSpread (the leftmost edge). As i increases, Direction rotates progressively
			 * toward the right. When i reaches NumRotators-1 (the last iteration), Direction will be rotated by
			 * DeltaSpread * (NumRotators-1) = Spread degrees from LeftOfSpread, placing it at the rightmost edge.
			 * This creates a linear progression of direction vectors spanning the entire spread angle, suitable
			 * for multi-projectile spread patterns where each projectile needs a unique firing direction.
			 */
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);

			/*
			 * Convert the Direction vector to an FRotator using the Rotation() function and add it to the Rotators
			 * array. Rotation() is an FVector member function that interprets the vector as a directional vector
			 * and converts it to the equivalent rotator representation (Pitch, Yaw, Roll). This conversion is
			 * necessary because rotators are the standard way to represent orientation in Unreal Engine for actors
			 * and components. The Add() function appends this rotator to the end of the Rotators TArray, building
			 * an array of evenly spaced rotators that can be used to spawn multiple projectiles, create
			 * spread attack patterns, or distribute any rotational effect across a defined angular range.
			 */
			Rotators.Add(Direction.Rotation());
		}
	}
	// If NumRotators is less than or equal to 1
	else
	{
		/*
		 * Convert the Forward vector directly to an FRotator using the Rotation() member function and add it to
		 * the Rotators array. Since only one rotator is requested (NumRotators <= 1), we don't need any spread
		 * calculations or angular distribution logic. We simply return the Forward direction as a single rotator,
		 * which represents the original direction without any angular offset. This handles edge cases where an
		 * ability or system requests a single projectile direction, ensuring the function always returns a valid
		 * array with at least one rotator element, even when no spread pattern is needed.
		 */
		Rotators.Add(Forward.Rotation());
	}
	/*
	 * Return the populated Rotators array to the calling code. This array contains either: a single rotator
	 * matching the Forward direction (if NumRotators <= 1), or multiple evenly spaced rotators distributed across
	 * the specified Spread angle (if NumRotators > 1). The returned array can be used by projectile spawning
	 * systems to create spread attack patterns, multi-shot abilities, or any gameplay mechanic that requires
	 * multiple evenly distributed directional vectors. TArray is returned by value, but Unreal's move semantics
	 * optimize this to avoid unnecessary copying, making the return efficient even for larger arrays.
	 */
	return Rotators;
}

TArray<FVector> UFoxAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumVectors)
{
	/*
	 * Declare an empty TArray of FVector to store the resulting direction vectors that will be evenly spaced
	 * across the specified spread angle. This array will be populated in the following logic and returned to
	 * the caller. Unlike EvenlySpacedRotators which returns FRotator objects, this function returns FVector
	 * direction vectors directly, which can be more efficient when the caller needs vectors for calculations
	 * rather than rotators for actor/component orientation. TArray is Unreal's dynamic array container that
	 * can grow/shrink as needed, making it ideal for storing a variable number of vectors based on the
	 * NumVectors input parameter. FVector is Unreal's structure for representing 3D vectors using three
	 * float components: X, Y, and Z, commonly used for directions, positions, velocities, and forces.
	 */
	TArray<FVector> Vectors;

	/*
	 * Calculate the leftmost edge of the spread angle by rotating the Forward vector by negative half the spread
	 * angle around the specified Axis. RotateAngleAxis returns a new FVector rotated by the given angle (in degrees)
	 * around the axis vector. By using -Spread / 2.f, we position this vector at the far left of the spread range.
	 * For example, if Spread is 90 degrees, this creates a vector rotated -45 degrees from Forward, establishing
	 * the starting point for distributing vectors across the spread. This leftmost position will be used as the
	 * base for incrementally rotating to create evenly spaced direction vectors in the subsequent loop.
	 */
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	
	/*
	 * Check if more than one vector is requested. If NumVectors is 1 or less, we don't need spacing calculations
	 * and can simply return the Forward vector directly. This conditional branching is necessary because the
	 * spacing calculation formula (Spread / (NumVectors - 1)) would result in division by zero if NumVectors is 1.
	 * When multiple vectors are needed, we proceed with the evenly-spaced distribution logic. Otherwise, we skip
	 * to the else block which handles the single vector case by directly adding the Forward vector to the array.
	 */
	if (NumVectors > 1)
	{
		/*
		 * Calculate the angular increment (in degrees) between each consecutive vector by dividing the total spread
		 * angle by the number of gaps between vectors. The number of gaps is always one less than the number of
		 * vectors (NumVectors - 1). For example, if Spread is 90 degrees and NumVectors is 5, DeltaSpread will
		 * be 90 / (5-1) = 22.5 degrees, meaning each vector will be spaced 22.5 degrees apart. This ensures the
		 * vectors are evenly distributed across the entire spread range from the leftmost to rightmost edge. 
		 */
		const float DeltaSpread = Spread / (NumVectors - 1);
		
		/*
		 * Iterate through each vector index from 0 to NumVectors - 1 (due to 0 based indexing the last element
		 * in the array is located at the NumVectors - 1 index) to generate evenly spaced direction vectors across
		 * the spread angle.
		 */
		for (int32 i = 0; i < NumVectors; i++)
		{
			/*
			 * Calculate a new direction vector by rotating the leftmost spread position (LeftOfSpread) by an
			 * incrementing angle around the specified Axis. The rotation angle is DeltaSpread multiplied by the
			 * current loop index i, which creates evenly spaced rotations across the spread range. When i=0,
			 * Direction equals LeftOfSpread (the leftmost edge). As i increases, Direction rotates progressively
			 * toward the right. When i reaches NumVectors-1 (the last iteration), Direction will be rotated by
			 * DeltaSpread * (NumVectors-1) = Spread degrees from LeftOfSpread, placing it at the rightmost edge.
			 * This creates a linear progression of direction vectors spanning the entire spread angle.
			 */
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			
			/*
			 * Add the calculated Direction vector directly to the Vectors array. Unlike EvenlySpacedRotators which
			 * converts vectors to rotators before adding them, this function stores the direction vectors in their
			 * native FVector format. This is more efficient when the calling code needs directional data for
			 * calculations like projectile velocities, forces, or raycasts, as it avoids the overhead of converting
			 * between rotators and vectors. The Add() function appends this vector to the end of the Vectors TArray,
			 * building an array of evenly spaced direction vectors.
			 */
			Vectors.Add(Direction);
		}
	}
	// If NumVectors is less than or equal to 1
	else
	{
		/*
		 * Add the Forward vector directly to the Vectors array without any rotation or conversion. Since only one
		 * vector is requested (NumVectors <= 1), we don't need any spread calculations or angular distribution logic.
		 * We simply return the Forward direction as a single vector, which represents the original direction without
		 * any angular offset. This handles edge cases where an ability or system requests a single projectile
		 * direction, ensuring the function always returns a valid array with at least one vector element, even when
		 * no spread pattern is needed.
		 */
		Vectors.Add(Forward);
	}
	/*
	 * Return the populated Vectors array to the calling code. This array contains either: a single vector matching
	 * the Forward direction (if NumVectors <= 1), or multiple evenly spaced direction vectors distributed across
	 * the specified Spread angle (if NumVectors > 1). The returned array can be used by projectile spawning systems
	 * to create spread attack patterns, multi-shot abilities, or any gameplay mechanic that requires multiple evenly
	 * distributed directional vectors. Unlike EvenlySpacedRotators which returns FRotator objects, this function
	 * returns FVector objects directly, which is more efficient for systems that need directional data for physics
	 * calculations, velocity assignments, or raycasting without requiring rotator-to-vector conversions. TArray is
	 * returned by value, but Unreal's move semantics optimize this to avoid unnecessary copying, making the return
	 * efficient even for larger arrays.
	 */
	return Vectors;
}
