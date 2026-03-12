#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Gameplay Camera (M2A)
// First-person camera locked to character eye
// position. Mouse look with pitch clamping.
// Produces CameraData for the renderer.
//
// No head bob, no smoothing, no lag.
// Raw and responsive for M2A.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/render/CameraData.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace freak {

class GameplayCamera {
public:
    /// Initialize with starting look direction.
    void Init(f32 yaw = -90.0f, f32 pitch = 0.0f);

    /// Update look direction from mouse delta.
    /// eyePosition comes from the FPS controller.
    void Update(const glm::vec3& eyePosition, f32 lookDeltaX, f32 lookDeltaY);

    /// Set aspect ratio (on window resize).
    void SetAspect(f32 aspect);

    /// Produce camera data for the renderer.
    [[nodiscard]] CameraData GetCameraData() const;

    /// Camera look direction (for interaction raycast).
    [[nodiscard]] const glm::vec3& Forward() const { return m_forward; }

    /// Current yaw in degrees (needed by FPSController for movement direction).
    [[nodiscard]] f32 Yaw() const { return m_yaw; }

    /// Current pitch in degrees (for debug display).
    [[nodiscard]] f32 Pitch() const { return m_pitch; }

    /// Mouse sensitivity in degrees per pixel.
    void SetSensitivity(f32 sensitivity) { m_sensitivity = sensitivity; }

private:
    void UpdateVectors();

    f32 m_yaw   = -90.0f;
    f32 m_pitch = 0.0f;

    glm::vec3 m_eyePosition{0.0f, 1.6f, 0.0f};
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};

    f32 m_sensitivity = 0.1f;
    f32 m_fov         = 70.0f;
    f32 m_aspect      = 16.0f / 9.0f;
    f32 m_nearClip    = 0.1f;
    f32 m_farClip     = 500.0f;
};

} // namespace freak
