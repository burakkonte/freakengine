#pragma once

// ─────────────────────────────────────────────
// Freak Engine - App lifecycle
// Handles SDL init/shutdown and owns the main loop timing.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

namespace freak {

// Initialize SDL and all subsystems. Call once at program start.
[[nodiscard]] Status AppInit();

// Shutdown SDL. Call once before exit.
void AppShutdown();

} // namespace freak
