#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Assertions
// Crash early, crash loud. Assertions are documentation that bites.
// ─────────────────────────────────────────────

#include <cstdio>
#include <cstdlib>

#ifdef FREAK_ASSERTS_ENABLED

    #ifdef FREAK_PLATFORM_WINDOWS
        #define FREAK_DEBUG_BREAK() __debugbreak()
    #else
        #include <csignal>
        #define FREAK_DEBUG_BREAK() raise(SIGTRAP)
    #endif

    #define FREAK_ASSERT(condition)                                              \
        do {                                                                     \
            if (!(condition)) {                                                  \
                std::fprintf(stderr, "[FREAK ASSERT] %s\n  File: %s\n  Line: %d\n", \
                    #condition, __FILE__, __LINE__);                             \
                FREAK_DEBUG_BREAK();                                             \
                std::abort();                                                    \
            }                                                                    \
        } while (false)

    #define FREAK_ASSERT_MSG(condition, msg)                                     \
        do {                                                                     \
            if (!(condition)) {                                                  \
                std::fprintf(stderr, "[FREAK ASSERT] %s\n  Message: %s\n  File: %s\n  Line: %d\n", \
                    #condition, (msg), __FILE__, __LINE__);                      \
                FREAK_DEBUG_BREAK();                                             \
                std::abort();                                                    \
            }                                                                    \
        } while (false)

    // Marks code paths that should never be reached
    #define FREAK_UNREACHABLE()                                                  \
        do {                                                                     \
            std::fprintf(stderr, "[FREAK ASSERT] Unreachable code reached\n  File: %s\n  Line: %d\n", \
                __FILE__, __LINE__);                                             \
            FREAK_DEBUG_BREAK();                                                 \
            std::abort();                                                        \
        } while (false)

#else
    #define FREAK_ASSERT(condition)         ((void)0)
    #define FREAK_ASSERT_MSG(condition, msg) ((void)0)
    #define FREAK_UNREACHABLE()             __assume(false)
#endif
