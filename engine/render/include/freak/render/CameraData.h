#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Camera Data
// Plain data produced by any camera implementation.
// This is the contract between cameras and the renderer.
// No virtuals, no inheritance — just data.
// ─────────────────────────────────────────────

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace freak {

/// View/projection matrices and world position.
/// Any camera type produces this; the renderer consumes it.
struct CameraData {
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};
    glm::vec3 position{0.0f};
};

} // namespace freak
