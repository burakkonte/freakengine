#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Scene Lighting Parameters
// Data-only struct describing the scene's global
// lighting and fog. Lives in Scene module because
// it's loaded from the scene file. Render reads it.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <glm/vec3.hpp>

namespace freak {

struct SceneLighting {
    // Directional light (moonlight)
    glm::vec3 lightDir{-0.3f, -0.7f, -0.5f};
    glm::vec3 lightColor{0.6f, 0.65f, 0.9f};
    f32 lightIntensity = 0.8f;
    glm::vec3 ambientColor{0.03f, 0.02f, 0.05f};

    // Fog
    glm::vec3 fogColor{0.05f, 0.02f, 0.08f};
    f32 fogDensity = 0.04f;
    f32 fogStart   = 5.0f;
    f32 fogEnd     = 80.0f;
};

} // namespace freak
