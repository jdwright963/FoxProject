#include "FoxLogChannels.h"

// Defines the actual log category "LogFox" that was forward-declared in FoxLogChannels.h.
// This macro (from Unreal Engine's LogMacros.h) creates the static data members and implementation
// needed for the log category to function. This definition must appear exactly once across all
// translation units (.cpp files) in the project, while the header's DECLARE_LOG_CATEGORY_EXTERN
// allows other files to reference it. Together, these macros enable project-wide logging with
// statements like: UE_LOG(LogFox, Warning, TEXT("message"))
DEFINE_LOG_CATEGORY(LogFox);