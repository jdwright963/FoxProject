// Copyright TryingToMakeGames


#include "AbilitySystem/Data/AttributeInfo.h"

#include "Fox/FoxLogChannels.h"

// Finds which index in the data asset (this) has the same tag as the AttributeTag input parameter
FFoxAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	
	// Iterate through each FFoxAttributeInfo element in the AttributeInformation array to search for an attribute matching the input tag.
	// Using a range-based for loop with const reference for efficiency (avoids copying the struct on each iteration).
	// This loop examines every entry until a match is found or the array is exhausted.
	for (const FFoxAttributeInfo& Info : AttributeInformation)
	{
		// Check if the current Info's AttributeTag exactly matches the AttributeTag input parameter we're searching for.
		// MatchesTagExact() performs a precise comparison - it only returns true if the tags are identical.
		// This is stricter than MatchesTag() which would also match parent tags in the hierarchy.
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			// Found a match. Return this FFoxAttributeInfo struct immediately, exiting the function.
			// This means the search stops as soon as the first matching entry is found.
			// The caller receives a copy of the matching attribute info with all its metadata (name, description, value).
			return Info;
		}
	}

	// If we reach this point, no matching attribute was found in the entire array.
	// Check if the caller requested logging for missing attributes (bLogNotFound parameter).
	// This conditional logging allows callers to suppress error messages when searching for optional attributes.
	if (bLogNotFound)
	{
		// Log an error message to help with debugging missing attribute configurations.
		// Uses the custom LogFox category defined in FoxLogChannels.h/cpp.
		// 
		// The * dereference operators are required to convert FString and FName objects to TCHAR* pointers.
		// UE_LOG's %s format specifier expects raw C-style string pointers (TCHAR*), not Unreal's string wrapper classes.
		// - AttributeTag.ToString() returns an FString object
		// - GetNameSafe(this) returns an FName object
		// Both FString and FName override the * operator to return their underlying TCHAR* data pointer,
		// enabling them to be used directly in printf-style formatting functions like UE_LOG.
		//
		// GetNameSafe() is a null-safe utility function that returns the object's name if valid, or "None" if the pointer is null.
		// This prevents crashes when logging with potentially null objects. Regular GetName() would crash if called on nullptr.
		// In this case, 'this' should never be null, but using GetNameSafe is defensive programming best practice.
		//
		// Format: "Can't find Info for AttributeTag [TagName] on AttributeInfo [DataAssetName]"
		UE_LOG(LogFox, Error, TEXT("Can't find Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	// Return an empty/default-constructed FFoxAttributeInfo struct to indicate no match was found.
	// This prevents crashes by providing a valid return value even when the search fails.
	// The caller should check if the returned struct has valid data (e.g., non-empty AttributeTag or AttributeName).
	// All members of the default struct will be initialized to their default values (empty tag, empty text, 0.0f value).
	return FFoxAttributeInfo();
}
