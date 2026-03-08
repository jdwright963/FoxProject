// Copyright TryingToMakeGames


#include "AbilitySystem/Data/AbilityInfo.h"

#include "Fox/FoxLogChannels.h"

FFoxAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	
	// Iterate through each FFoxAbilityInfo element in the AbilityInformation array to search for an ability matching the input tag.
	// Using a range-based for loop with const reference for efficiency (avoids copying the struct on each iteration).
	for (const FFoxAbilityInfo& Info : AbilityInformation)
	{
		// Check if the current Info's AbilityTag exactly matches the AbilityTag input parameter we're searching for.
		// The == operator for FGameplayTag performs an exact match comparison.
		if (Info.AbilityTag == AbilityTag)
		{
			// Found a match. Return this FFoxAbilityInfo struct immediately, exiting the function.
			// This means the search stops as soon as the first matching entry is found.
			return Info;
		}
	}

	// If we reach this point, no matching ability was found in the entire array.
	// Check if the caller requested logging for missing abilities (bLogNotFound parameter).
	if (bLogNotFound)
	{
		// Log an error message to help with debugging missing ability configurations.
		// Uses the custom LogFox category defined in FoxLogChannels.h/cpp.
		// 
		// The * dereference operators are required to convert FString and FName objects to TCHAR* pointers.
		// UE_LOG's %s format specifier expects raw C-style string pointers (TCHAR*), not Unreal's string wrapper classes.
		// - AbilityTag.ToString() returns an FString object
		// - GetNameSafe(this) returns an FName object
		// Both FString and FName override the * operator to return their underlying TCHAR* data pointer,
		// enabling them to be used directly in printf-style formatting functions like UE_LOG.
		//
		// GetNameSafe() is a null-safe utility function that returns the object's name if valid, or "None" if the pointer is null.
		// This prevents crashes when logging with potentially null objects. Regular GetName() would crash if called on nullptr.
		// In this case, 'this' should never be null, but using GetNameSafe is defensive programming best practice.
		//
		// Format: "Can't find info for AbilityTag [TagName] on AbilityInfo [DataAssetName]"
		UE_LOG(LogFox, Error, TEXT("Can't find info for AbilityTag [%s] on AbilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}

	// Return an empty/default-constructed FFoxAbilityInfo struct to indicate no match was found.
	// This prevents crashes by providing a valid return value even when the search fails.
	// The caller should check if the returned struct has valid data (e.g., non-empty AbilityTag).
	return FFoxAbilityInfo();
}
