// Copyright TryingToMakeGames


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"
#include "Fox/Fox.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	// Needs comment
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	
	
	/*
	 * Checks if we are locally controlled. This check is primarily designed for dedicated server scenarios where only
	 * clients should send target data to the server for their locally controlled player. In that case, IsLocallyControlled()
	 * returns true on the client that owns the player, and false on the server (since the server doesn't locally control
	 * any player in a dedicated server setup).
	 *
	 * However, in a listen server scenario, the server acts as both server and client for one player. For that specific
	 * player (the one the listen server is playing as), IsLocallyControlled() returns true on the server. This means
	 * SendMouseCursorData() will be called on the server, which might seem redundant since we're already on the server.
	 * Despite this, the function still works correctly because:
	 * 1. The FScopedPredictionWindow handles the prediction key setup appropriately for local server execution
	 * 2. ServerSetReplicatedTargetData is designed to handle being called locally on the server
	 * 3. The ValidData delegate broadcast still triggers the ability graph correctly
	 *
	 * For other players connected to a listen server (not the host), IsLocallyControlled() returns false on the server,
	 * so the server-side delegate binding path (the else branch) is used to wait for their target data from clients.
	 *
	 * In summary: this check ensures clients send their target data to the server, and in listen server mode, it also
	 * allows the server's own player to "send" data to itself, which the system handles gracefully.
	*/
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		// Calls the function to send the target data (mouse cursor location) to the server
		SendMouseCursorData();
	}
	
	// If not locally controlled, we are on the server so listen for target data 
	else
	{
		// Create a variable for the ability spec handle and activation prediction key and get them
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		
		/*
		 * Binds our callback function OnTargetDataReplicatedCallback to the Ability System Component's target data delegate
		 * for this specific ability instance. AbilityTargetDataSetDelegate is a function that returns a reference to a 
		 * multicast delegate associated with the given SpecHandle and ActivationPredictionKey. This delegate will be 
		 * broadcast when the server receives target data from the client via ServerSetReplicatedTargetData RPC.
		 *
		 * AddUObject binds a UObject member function to the delegate. It takes two parameters: 'this' (a pointer to the 
		 * current UTargetDataUnderMouse instance that will receive the callback) and a pointer to the member function 
		 * &UTargetDataUnderMouse::OnTargetDataReplicatedCallback that will be invoked when target data arrives.
		 *
		 * This binding is essential for the server-side flow: when a client sends mouse cursor data through 
		 * ServerSetReplicatedTargetData, the server's Ability System Component will broadcast this delegate, triggering
		 * our OnTargetDataReplicatedCallback function so we can process the received target data and continue ability
		 * execution on the server with the client's targeting information.
		*/
		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);

		/*
		 * Attempts to immediately call any already-set replicated target data delegates for this ability instance.
		 * CallReplicatedTargetDataDelegatesIfSet checks if target data has already been received and cached by the
		 * Ability System Component for the given SpecHandle and ActivationPredictionKey. This can happen in cases
		 * where the client's target data RPC arrived at the server before this Activate() function was called.
		 *
		 * If cached target data exists, this function immediately invokes the delegate we just bound (triggering
		 * OnTargetDataReplicatedCallback) and returns true. If no cached data exists, it returns false, meaning
		 * we need to wait for the target data to arrive from the client at a later time. This pattern prevents
		 * race conditions and ensures we don't miss target data regardless of network timing.
		*/
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

		/*
		 * Checks if the delegate was NOT called (meaning no cached target data was available). If bCalledDelegate is
		 * false, it means CallReplicatedTargetDataDelegatesIfSet did not find any pre-existing target data, so we are
		 * still waiting for the client to send the target data via RPC.
		 *
		 * In this case, we call SetWaitingOnRemotePlayerData() to mark this ability task as waiting for remote player
		 * data. This is important for the ability system's internal state management—it tells the system that this task
		 * cannot complete until data arrives from a remote client. The task will remain in a waiting state until the
		 * client sends the target data through ServerSetReplicatedTargetData, which will then trigger our bound callback
		 * OnTargetDataReplicatedCallback and allow the ability to continue execution on the server.
		*/
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	/*
	 * Creates a scoped prediction window for client-side prediction of ability system operations. FScopedPredictionWindow
	 * is a RAII (Resource Acquisition Is Initialization) class that manages prediction keys for the Gameplay Ability System.
	 * When constructed, it opens a new prediction window on the Ability System Component, generating a prediction key that
	 * allows the client to predict ability outcomes locally before receiving server confirmation. This enables responsive
	 * gameplay by allowing the client to immediately show effects while waiting for server validation. When the 
	 * FScopedPredictionWindow object goes out of scope (at the end of this function), its destructor automatically closes
	 * the prediction window and cleans up the prediction key. The AbilitySystemComponent.Get() call dereferences the weak
	 * pointer to get the raw UAbilitySystemComponent pointer needed for the prediction window.
	*/
	// In other words this asks the server to allow us to do what we do in the rest of this function locally on the client.
	// Then, the server will do it when the server knows about it.
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	/*
	 * Ability tasks have an Ability member variable for the ability they belong to which has a GetCurrentActorInfo() function
	 * that returns a FGameplayAbilityActorInfo* which is cached data associated with an Actor using an Ability. This type
	 * has a member variable PlayerController that returns a TWeakObjectPtr<APlayerController> which is a weak reference 
	 * to the player controller associated with it. The Get function dereferences the weak pointer to retrieve the actual
	 * APlayerController pointer
	*/
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	
	/*
	 * Declares a variable of type FHitResult to store the results of a line trace (raycast) from the mouse cursor.
	 * FHitResult is an Unreal Engine struct that contains comprehensive information about a trace/raycast hit, including:
	 *   - bBlockingHit: Boolean indicating whether the trace hit a blocking object (solid geometry)
	 *   - ImpactPoint: The exact world location (FVector) where the trace intersected with the hit object
	 *   - Location: Alias for ImpactPoint, the world space location of the hit
	 *   - Normal: The surface normal vector at the point of impact (perpendicular to the surface)
	 *   - ImpactNormal: Alias for Normal, used to determine surface orientation
	 *   - Actor: A weak pointer to the AActor that was hit by the trace
	 *   - Component: The specific UPrimitiveComponent within the actor that was hit
	 *   - BoneName: If the hit component is skeletal mesh, the name of the bone that was hit
	 *   - Distance: The distance from the trace start point to the hit location
	 *   - Time: A normalized value (0.0 to 1.0) representing how far along the trace the hit occurred
	 * This variable will be populated by GetHitResultUnderCursor with the results of tracing from the camera
	 * through the mouse cursor position into the world, allowing us to determine what the player is pointing at.
	*/
	FHitResult CursorHit;

	/*
	 * Performs a line trace (raycast) from the camera through the screen position of the mouse cursor into the 3D world.
	 * GetHitResultUnderCursor is a PlayerController function that internally:
	 *   1. Gets the current 2D screen position of the mouse cursor
	 *   2. Deprojects that screen position into a 3D world ray (origin and direction) using the camera's view parameters
	 *   3. Performs a line trace along that ray to find the first object the cursor is pointing at
	 *   4. Populates the output parameter with hit information if something was hit
	 *
	 * Parameters breakdown:
	 * 
	 * 1. ECC_Target: The collision channel to trace against, defined in Fox.h as ECC_GameTraceChannel2. This is a custom
	 *    collision channel configured in the project settings that determines which types of objects this trace can detect.
	 *    Only objects that are set to block or overlap the ECC_Target channel will be detected by this trace. This allows
	 *    us to filter the trace to only hit targetable objects (enemies, interactive objects, etc.) while ignoring other
	 *    geometry like scenery or non-interactive props that might not have this channel enabled.
	 *
	 * 2. false: The bTraceComplex parameter, which determines the precision of the collision trace. When false (simple collision),
	 *    the trace uses simplified collision shapes (boxes, spheres, capsules) that designers set up for performance. When true
	 *    (complex collision), the trace tests against the actual polygon mesh geometry, which is much more accurate but significantly
	 *    more expensive computationally. We use false here for better performance since simple collision is sufficient for most
	 *    gameplay targeting scenarios and provides consistent, predictable hit detection.
	 *
	 * 3. CursorHit: An output parameter (passed by reference) that will be filled with the hit result data. If the trace hits
	 *    something, CursorHit.bBlockingHit will be true and the struct will contain information about what was hit (actor,
	 *    location, normal, etc.). If nothing is hit, bBlockingHit will be false. This is the FHitResult variable we declared
	 *    above, and after this function call it will contain all the targeting information we need for the ability.
	 *
	 * The function returns a boolean indicating whether a blocking hit occurred, but we don't capture that return value here
	 * because we can check CursorHit.bBlockingHit directly, which provides the same information and is more explicit in the code.
	*/
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);
	
	// Creates a variable to hold the target data handle
	FGameplayAbilityTargetDataHandle DataHandle;
	
	/*
	 * Creates a new instance of FGameplayAbilityTargetData_SingleTargetHit on the heap using the new operator.
	 * FGameplayAbilityTargetData is the base struct used by the Gameplay Ability System to represent targeting
	 * information—it encapsulates data about what an ability is targeting (such as actors or hit results).
	 * This allows abilities to pass data between client and server in a replicated, polymorphic way.
	 *
	 * In C++, the "stack" is a fast, temporary memory area for local variables that are destroyed the moment
	 * a function ends. The "heap" is a larger pool of memory for data that needs to stay alive longer. We
	 * allocate this on the heap (using "new") because targeting data must persist after this function ends
	 * so it can be processed by the ability system or sent over the network to the server.
	 *
	 * FGameplayAbilityTargetData_SingleTargetHit is a derived struct that specifically stores target data
	 * for a single hit result. This makes it ideal for scenarios like this where we need to communicate a
	 * single raycast or line trace result from the client's mouse cursor to the server.
	 *
	 * This heap allocation is required for "polymorphism." Think of the General type as a small storage 
	 * box and the Specific type as a large storage box. "Slicing" is what happens if you try to force 
	 * the large box into the small one; C++ literally chops off the extra data to make it fit. This 
	 * would destroy our "Hit Result" info because the General type doesn't have a place to store it.
	 *
	 * By using a heap pointer, we aren't moving boxes, we are just passing a memory address. No matter 
	 * how big the Specific data is, an address is always the same size. We give the system a General 
	 * address, but when the system follows that address to the heap, the full, Specific object is 
	 * still sitting there entirely intact. This allows the system to treat our data as "General" 
	 * for organization, while still keeping all the "Specific" details safe and accessible.
	 *
	 * We use the heap because the DataHandle container takes "ownership" of the pointer and manages its
	 * lifetime. The FGameplayAbilityTargetDataHandle will eventually delete this object when it is no 
	 * longer needed, so we do not need to manually delete it ourselves. The use of "new" is required as 
	 * the DataHandle.Add() function specifically expects a heap pointer that it can safely take over.
	*/
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	
	// Assigns the hit result to the target data
	Data->HitResult = CursorHit;
	
	// Adds a new target data to handle, it must have been created with new
	DataHandle.Add(Data);
	
	
	/*
	 * Sends the target data we collected on the client to the server through a server RPC (Remote Procedure Call).
	 * ServerSetReplicatedTargetData is a server function in UAbilitySystemComponent that receives target data from
	 * clients and processes it on the server for gameplay ability execution. This is essential in a client-server
	 * architecture because the server is authoritative and must validate and process all gameplay-affecting data.
	 *
	 * Parameters breakdown:
	 * 
	 * 1. GetAbilitySpecHandle(): Returns the FGameplayAbilitySpecHandle that uniquely identifies this specific
	 *	instance of the ability that is currently executing. This tells the server which ability activation
	 *	this target data belongs to, ensuring the data is associated with the correct ability instance.
	 *
	 * 2. GetActivationPredictionKey(): Returns the FPredictionKey that was assigned when this ability was
	 *	activated. Prediction keys are used to match client-side predicted actions with their server-side
	 *	confirmations. This allows the server to correlate this target data with the predicted ability
	 *	activation that occurred on the client, enabling proper rollback or confirmation of predictions.
	 *
	 * 3. DataHandle: The FGameplayAbilityTargetDataHandle containing our mouse cursor hit result that we
	 *	constructed earlier in this function. This is the actual payload being sent to the server. It is the
	 *	targeting information the ability needs to execute (in this case, where the player clicked).
	 *
	 * 4. FGameplayTag(): An empty gameplay tag passed as a parameter. This can be used to provide additional
	 *	context or categorization for the target data, but in this case we don't need any special tagging
	 *	so we pass a default-constructed (empty) tag.
	 *
	 * 5. AbilitySystemComponent->ScopedPredictionKey: The current scoped prediction key from the Ability
	 *	System Component. This is the prediction key that was set up by our FScopedPredictionWindow earlier
	 *	in this function. It's used to maintain the prediction context when the server processes this RPC,
	 *	ensuring the server can properly handle the predicted gameplay effects and maintain synchronization
	 *	between client predictions and server authority.
	 *
	 * After this call executes, the server will have received our targeting data and can use it to execute
	 * the ability authoritatively, while our client-side prediction (set up by FScopedPredictionWindow) allows
	 * us to show immediate feedback to the player without waiting for server round-trip time.
	*/
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(), 
		GetActivationPredictionKey(), 
		DataHandle, 
		FGameplayTag(), 
		AbilitySystemComponent->ScopedPredictionKey);
		
	// This should be called prior to broadcasting delegates back into the ability graph. This makes sure the ability 
	// is still active.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// Broadcast the FGameplayAbilityTargetDataHandle DataHandle that contains the HitResult that contains the location
		// (FVector) of the hit result which is the mouse cursor location. This broadcast causes the ValidData execution 
		// pin on the TargetDataUnderMouse node in the Blueprint to be executed and the Data node to have the value from 
		// DataHandle which contains the mouse cursor location
		ValidData.Broadcast(DataHandle);
	}
}

// Callback for when target data is replicated from the client to the server
void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	// Tells the ability system component that the server has received the target data and that the ASC does not need
	// to store this TargetData anymore. The ASC uses pairs of AbilitySpecHandles and ActivationPrediction keys mapped to 
	// TargetData stored in FAbilityReplicatedDataCache to keep track of which target data is for which ability spec and
	// activation prediction key
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	
	// This should be called prior to broadcasting delegates back into the ability graph. This makes sure the ability 
	// is still active.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// Broadcast the FGameplayAbilityTargetDataHandle DataHandle that contains the HitResult that contains the location
		// (FVector) of the hit result which is the mouse cursor location. This broadcast causes the ValidData execution 
		// pin on the TargetDataUnderMouse node in the Blueprint to be executed and the Data node to have the value from 
		// DataHandle which contains the mouse cursor location
		ValidData.Broadcast(DataHandle);
	}
}
