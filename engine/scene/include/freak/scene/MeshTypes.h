#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Mesh type definitions
// Shared between Scene (components) and Render (mesh cache).
// Lives in Scene module because it's a component-level type.
// Has zero dependencies beyond core types.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

namespace freak {

// Lightweight handle — an index into whatever mesh storage exists.
// Scene components store this (resolved at load time).
// Render reads it to look up GPU buffers.
enum class MeshID : u16 {
    Invalid = 0xFFFF
};

} // namespace freak
