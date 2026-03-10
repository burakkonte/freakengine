#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Vertex Layout
// Single vertex format for all world geometry in M1.
// Position + Normal + Color (RGBA u8).
// UV will be added when textures arrive (M1.5).
// This layout is the contract between MeshCache
// and the forward shader — they must match exactly.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <bgfx/bgfx.h>
#include <glm/vec3.hpp>

namespace freak {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    u32 color; // ABGR packed (bgfx convention)

    static void Init() {
        s_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,   3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .end();
    }

    static bgfx::VertexLayout s_layout;
};

// Pack RGBA floats [0,1] into ABGR u32 (bgfx convention)
inline u32 PackColor(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    auto clamp = [](f32 v) -> u8 {
        return static_cast<u8>(v < 0.0f ? 0.0f : (v > 1.0f ? 255.0f : v * 255.0f));
    };
    return (static_cast<u32>(clamp(a)) << 24)
         | (static_cast<u32>(clamp(b)) << 16)
         | (static_cast<u32>(clamp(g)) << 8)
         | (static_cast<u32>(clamp(r)));
}

} // namespace freak
