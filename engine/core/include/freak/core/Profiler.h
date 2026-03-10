#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Profiler macros (Tracy wrapper)
// Wraps Tracy so the rest of the codebase never
// includes Tracy directly. If we ever swap profilers,
// only this file changes.
// ─────────────────────────────────────────────

#ifdef TRACY_ENABLE
    #include <tracy/Tracy.hpp>

    // Mark a frame boundary — call once per frame in the main loop
    #define FREAK_FRAME_MARK          FrameMark

    // CPU zone — place at the top of a function or scope
    #define FREAK_PROFILE_SCOPE       ZoneScoped
    #define FREAK_PROFILE_SCOPE_N(name) ZoneScopedN(name)

    // Named zone with color (for distinguishing render passes, etc.)
    #define FREAK_PROFILE_SCOPE_C(name, color) \
        ZoneScopedN(name); ZoneColor(color)

    // Plot a value (e.g., entity count, draw call count)
    #define FREAK_PROFILE_PLOT(name, value) TracyPlot(name, value)

    // Memory tracking
    #define FREAK_PROFILE_ALLOC(ptr, size)  TracyAlloc(ptr, size)
    #define FREAK_PROFILE_FREE(ptr)         TracyFree(ptr)

    // Message logging visible in Tracy
    #define FREAK_PROFILE_MESSAGE(text, len) TracyMessage(text, len)

#else
    #define FREAK_FRAME_MARK
    #define FREAK_PROFILE_SCOPE
    #define FREAK_PROFILE_SCOPE_N(name)
    #define FREAK_PROFILE_SCOPE_C(name, color)
    #define FREAK_PROFILE_PLOT(name, value)
    #define FREAK_PROFILE_ALLOC(ptr, size)
    #define FREAK_PROFILE_FREE(ptr)
    #define FREAK_PROFILE_MESSAGE(text, len)
#endif
