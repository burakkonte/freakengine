#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Logging
// Simple, fast, no-allocation-in-hot-path logging.
// Uses std::format for type-safe formatting.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

#include <format>
#include <string_view>

namespace freak::log {

enum class Level : u8 {
    Trace,   // Spammy, disabled in develop+
    Debug,   // Useful during development
    Info,    // Normal operational messages
    Warn,    // Something is off but recoverable
    Error,   // Something failed
    Fatal,   // About to crash
};

// Initialize the logging system. Call once at startup.
void Init();

// Shutdown. Flushes any buffered output.
void Shutdown();

// Set the minimum level. Messages below this are discarded.
void SetLevel(Level minLevel);

// Core log function — prefer the macros below.
void LogMessage(Level level, std::string_view category, std::string_view message);

} // namespace freak::log

// ─────────────────────────────────────────────
// Logging macros — use these in code
// Category is a short string like "Render", "Audio", "Game"
// ─────────────────────────────────────────────

#define FREAK_LOG_TRACE(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Trace, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define FREAK_LOG_DEBUG(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Debug, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define FREAK_LOG_INFO(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Info, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define FREAK_LOG_WARN(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Warn, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define FREAK_LOG_ERROR(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Error, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define FREAK_LOG_FATAL(category, fmt, ...) \
    ::freak::log::LogMessage(::freak::log::Level::Fatal, category, std::format(fmt __VA_OPT__(,) __VA_ARGS__))
