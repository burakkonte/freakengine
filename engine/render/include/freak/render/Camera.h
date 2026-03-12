#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Free-fly Debug Camera
// WASD + mouse look, shift for speed boost, scroll
// to change base speed. This is a permanent dev tool,
// not the gameplay camera (which comes in M2).
//
// The camera produces a view matrix and projection
// matrix. It has no knowledge of the scene or entities.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/render/CameraData.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace freak {

class Input;

struct CameraState {
    glm::vec3 position{0.0f, 2.0f, 5.0f};
    f32 yaw   = -90.0f;  // Degrees, -90 = looking along -Z
    f32 pitch = -10.0f;  // Degrees, clamped to ±89
};

class DebugCamera {
public:
    void Init(const CameraState& state = {});

    // Process input and update matrices. Call once per frame.
    void Update(const Input& input, f64 deltaTime);

    // Recalculate projection when aspect ratio changes.
    void SetAspect(f32 aspect);

    // Access matrices for rendering
    [[nodiscard]] const glm::mat4& ViewMatrix() const { return m_view; }
    [[nodiscard]] const glm::mat4& ProjMatrix() const { return m_proj; }

    /// Produce a CameraData snapshot for the renderer.
    [[nodiscard]] CameraData GetCameraData() const;

    // Read camera state (for debug UI display)
    [[nodiscard]] const glm::vec3& Position() const { return m_state.position; }
    [[nodiscard]] f32 Yaw() const { return m_state.yaw; }
    [[nodiscard]] f32 Pitch() const { return m_state.pitch; }
    [[nodiscard]] f32 MoveSpeed() const { return m_baseSpeed; }

    // The forward direction vector (normalized)
    [[nodiscard]] const glm::vec3& Forward() const { return m_forward; }

    // Configuration
    void SetFOV(f32 fovDegrees);
    void SetClipPlanes(f32 near, f32 far);
    void SetSensitivity(f32 sensitivity);

private:
    void UpdateVectors();
    void UpdateViewMatrix();
    void UpdateProjMatrix();

    CameraState m_state;

    // Derived direction vectors
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    // Matrices
    glm::mat4 m_view{1.0f};
    glm::mat4 m_proj{1.0f};

    // Movement
    f32 m_baseSpeed    = 5.0f;     // Units per second
    f32 m_sprintMult   = 3.0f;     // Shift multiplier
    f32 m_smoothFactor = 12.0f;    // Acceleration smoothing
    glm::vec3 m_velocity{0.0f};

    // Look
    f32 m_sensitivity = 0.1f;      // Degrees per pixel of mouse movement

    // Projection
    f32 m_fovDegrees = 70.0f;
    f32 m_aspect     = 16.0f / 9.0f;
    f32 m_nearClip   = 0.1f;
    f32 m_farClip    = 500.0f;
};

} // namespace freak
