
#include "FoxAbilityTypes.h"

// Much of this code is very similar to the function we are overriding
bool FFoxGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// variable to hold the bits that indicate which variables to serialize, 1 (true) = bit is set, 0 (false) = bit is not set
	// The original FGameplayEffectContext uses uint8 here. We use uint32 because we need more bits to represent all the
	// variables we want to serialize.
	uint32 RepBits = 0;
	
	// Checks if the archive is being saved
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		// The if checks and their contents above this line were copied from the function we are overriding.
		// This line checks if bIsBlockedHit is true. If so, it should be serialized.
		if (bIsBlockedHit)
		{
			// Uses the bitwise OR and left shift (<<) operators to set (flip) the 8th bit of RepBits to 1. Indicating 
			// bIsBlockedHit should be serialized. Bitwise OR compares two bits and if either is a 1 the result is 1.
			// 1 (32 bit) is:     0000 0000 0000 0000 0000 0000 0000 0001
			// 1 << 7 is:         0000 0000 0000 0000 0000 0000 1000 0000
			// RepBits initially: 0000 0000 0000 0000 0000 0000 0000 0000
			// RepBits |= 1 << 7: 0000 0000 0000 0000 0000 0000 1000 0000
			RepBits |= 1 << 7;
		}
		// See comments above for similar code
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 13;
		}
		if (!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 14;
		}
		if (!KnockbackForce.IsZero())
		{
			RepBits |= 1 << 15;
		}
	}

	// Calls the SerializeBits function on the archive passing in a pointer (the address of) to RepBits and the number of bits used in 
	// RepBits to indicate variables to serialize. '1 << 13' is used to move the 1 bit to the 14th position.
	Ar.SerializeBits(&RepBits, 16);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		// This if statement checks the return value of IsLoading() function called on the Ar (archive) variable. IsLoading()
		// returns true when loading/deserializing data from the network, false otherwise. When the condition evaluates to true,
		// we need to ensure HitResult has a valid shared pointer before deserializing into it. We only need to initialize
		// the pointer when receiving data.
		if (Ar.IsLoading())
		{
			// Check if HitResult shared pointer is null or invalid.
			if (!HitResult.IsValid())
			{
				// If so, create a new FHitResult instance wrapped in a
				// TSharedPtr to provide a valid memory location for the upcoming NetSerialize call to deserialize data into.
				// Breaking down the syntax: TSharedPtr<FHitResult>(new FHitResult())
				// - 'new FHitResult()' allocates memory on the heap (dynamic memory) for an FHitResult object and calls its
				//   default constructor (the empty parentheses). The 'new' keyword returns a raw pointer to this heap-allocated object.
				// - TSharedPtr<FHitResult>(...) is calling TSharedPtr's constructor, which takes a raw pointer as an argument.
				//   We pass the raw pointer from 'new FHitResult()' into TSharedPtr's constructor parentheses so TSharedPtr can
				//   take ownership of the heap-allocated object and manage its lifetime (automatic cleanup when no longer referenced).
				// - Without 'new', FHitResult() would create a temporary stack object that gets destroyed immediately. With 'new',
				//   the object persists on the heap until TSharedPtr cleans it up, which is necessary for network deserialization.
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	// The code above this line and below `Ar.SerializeBits(&RepBits, 9);` were copied from the function we are overriding.
	
	// This line Uses the bitwise AND operator (&) to check if the 8th bit of RepBits is set to 1. If it is, bIsBlockedHit needs to be 
	// serialized. The expression (1 << 7) creates a mask with only the 8th bit set. Bitwise AND compares corresponding bits 
	// and returns 1 only if both bits are 1, otherwise 0. If the result is non-zero, the condition is true.
	// Example: RepBits = 0000 0000 0000 0000 0000 0000 1000 0000, (1 << 7) = 0000 0000 0000 0000 0000 0000 1000 0000
	// RepBits & (1 << 7) = 0000 0000 0000 0000 0000 0000 1000 0000 (non-zero, condition is true)
	if (RepBits & (1 << 7))
	{
		// Uses the << operator to serialize bIsBlockedHit. The << operator is overloaded in FArchive to handle serialization
		// of various data types. When Ar.IsSaving() is true, it writes bIsBlockedHit's value to the archive. When Ar.IsLoading()
		// is true, it reads the value from the archive and stores it in bIsBlockedHit. This bidirectional behavior allows the
		// same code to handle both saving and loading, automatically adapting based on the archive's mode.
		Ar << bIsBlockedHit;
	}
	// See comments above for very similar code
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDebuff;
	}
	if (RepBits & (1 << 10))
	{
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DebuffFrequency;
	}
	if (RepBits & (1 << 13))
	{
		// This if statement checks the return value of IsLoading() function called on the Ar (archive) variable. IsLoading()
		// returns true when loading/deserializing data from the network, false otherwise. When the condition evaluates to true,
		// we need to ensure DamageType has a valid shared pointer before deserializing into it. This is similar to how
		// HitResult is handled above. We only need to initialize the pointer when receiving data.
		if (Ar.IsLoading())
		{
			// Check if DamageType shared pointer is null or invalid. 
			if (!DamageType.IsValid())
			{
				// If so, create a new FGameplayTag instance wrapped in a
				// TSharedPtr to provide a valid memory location for the upcoming NetSerialize call to deserialize data into.
				// Breaking down the syntax: TSharedPtr<FGameplayTag>(new FGameplayTag())
				// - 'new FGameplayTag()' allocates memory on the heap (dynamic memory) for an FGameplayTag object and calls its
				//   default constructor (the empty parentheses). The 'new' keyword returns a raw pointer to this heap-allocated object.
				// - TSharedPtr<FGameplayTag>(...) is calling TSharedPtr's constructor, which takes a raw pointer as an argument.
				//   We pass the raw pointer from 'new FGameplayTag()' into TSharedPtr's constructor parentheses so TSharedPtr can
				//   take ownership of the heap-allocated object and manage its lifetime (automatic cleanup when no longer referenced).
				// - Without 'new', FGameplayTag() would create a temporary stack object that gets destroyed immediately. With 'new',
				//   the object persists on the heap until TSharedPtr cleans it up, which is necessary for network deserialization.
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		// Call NetSerialize on the DamageType to either write its value to the archive (when saving) or read the value from
		// the archive into the DamageType object (when loading). The -> operator dereferences the shared pointer to access
		// the underlying FGameplayTag's NetSerialize method.
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 14))
	{
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 15))
	{
		KnockbackForce.NetSerialize(Ar, Map, bOutSuccess);
	}
	// The following was copied from the function we are overriding
	// Checks if the archive is being loaded
	if (Ar.IsLoading())
	{
		// When loading from the archive, we need to reinitialize the InstigatorAbilitySystemComponent by calling AddInstigator().
		// This function takes the deserialized Instigator and EffectCauser actors (retrieved via Get() which returns the raw pointer
		// from the TWeakObjectPtr) and sets up the internal AbilitySystemComponent reference. This ensures the context has a valid
		// reference to the instigator's ability system after being received over the network.
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	

	// Set bOutSuccess to true to indicate that the serialization operation completed successfully. This out parameter is used by
	// the networking system to determine if the data was properly serialized/deserialized. Returning true from this function
	// along with bOutSuccess = true signals that the NetSerialize operation succeeded without errors.
	bOutSuccess = true;
	
	return true;
}
