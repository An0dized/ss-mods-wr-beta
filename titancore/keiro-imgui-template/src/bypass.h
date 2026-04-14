#pragma once
#include "pch.h"

namespace Bypass {
    // Performs the full Denuvo Anti-Cheat bypass sequence (blocking):
    //   1. Wait up to 60s for denuvo-anti-cheat-runtime.dll to load in this process
    //   2. Call stopServices() export from the runtime DLL (disables internal watchdog)
    //   3. Stop both Denuvo kernel services via the Service Control Manager
    //   4. Patch runtimeApiDriverHandshakeStatus -> always return 1 (SUCCESS)
    //   5. Delete leftover Denuvo log files
    // Returns true on success, false if the runtime DLL never appeared.
    bool Run();
}
