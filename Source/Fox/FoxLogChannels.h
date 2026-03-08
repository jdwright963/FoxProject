#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"


// Declares a custom log category named "LogFox" that can be used throughout the project for logging.
// This is a forward declaration in the header file - the actual definition is in the .cpp file using DEFINE_LOG_CATEGORY.
// We use the DECLARE_LOG_CATEGORY_EXTERN macro (provided by Unreal Engine's LogMacros.h) to create this logging category. 
// The macro expands to create the necessary extern declarations for the log category's static data members, making it 
// accessible throughout the codebase after being defined once in a .cpp file.
//
// Parameters:
// - LogFox: The name of the log category used in UE_LOG macros (e.g., UE_LOG(LogFox, Warning, TEXT("message")))
// - Log: The default verbosity level at compile time (options: Fatal, Error, Warning, Display, Log, Verbose, VeryVerbose)
//        This sets a compile-time filter that determines which log messages are compiled into the binary.
//        Any log statements with a verbosity level MORE verbose than this will be completely stripped out during compilation,
//        meaning they won't exist in the final executable and have zero runtime cost.
//        For example, if set to "Log", then Verbose and VeryVerbose messages won't be compiled at all.
//        This is useful for removing expensive debug logging from shipping builds while keeping the code in place.
// - All: The maximum verbosity level that can be set at runtime via console commands
//        This runtime limit allows you to filter logs dynamically without recompiling (e.g., "Log LogFox Verbose" in console).
//
// Usage example: UE_LOG(LogFox, Error, TEXT("Can't find Info for AttributeTag [%s]"), *AttributeTag.ToString());
DECLARE_LOG_CATEGORY_EXTERN(LogFox, Log, All);