#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Core type aliases
// Fixed-width types and common definitions.
// ─────────────────────────────────────────────

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace freak {

// Fixed-width integers
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;

// Common string types
using String     = std::string;
using StringView = std::string_view;

// Error handling via C++23 std::expected
// Usage: Result<T> myFunction();
//   auto result = myFunction();
//   if (result) { use *result; } else { handle result.error(); }
enum class Error {
    Unknown,
    FileNotFound,
    InvalidData,
    OutOfMemory,
    InitFailed,
    ShutdownFailed,
};

template<typename T>
using Result = std::expected<T, Error>;

// Unit result for operations that either succeed or fail
using Status = std::expected<void, Error>;

} // namespace freak
