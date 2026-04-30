// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define CUSTOM_DEPTH_RED 250

// Creates an alias for projectile collision channel we created in the editor project settings
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1

// Creates an alias for Target collision channel we created in the editor project settings
#define ECC_Target ECollisionChannel::ECC_GameTraceChannel2

// Creates an alias for ExcludePlayers collision channel we created in the editor project settings
#define ECC_ExcludePlayers ECollisionChannel::ECC_GameTraceChannel3