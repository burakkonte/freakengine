#pragma once

// ─────────────────────────────────────────────
// Freak Engine - First-Person Controller (M2A)
// Grounded character movement with AABB collision.
// Owns position and velocity. Produces eye position
// for the gameplay camera.
//
// Uses CollisionWorld::MoveAndSlide for collision.
// No physics simulation — purely kinematic.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <glm/vec3.hpp>

namespace freak {

struct InputActions;
class CollisionWorld;

struct FPSControllerConfig {
    f32 moveSpeed    = 5.0f;     // Units per second
    f32 sprintMult   = 2.0f;     // Multiplier when sprinting
    f32 jumpSpeed    = 5.5f;     // Initial upward velocity on jump
    f32 gravity      = 15.0f;    // Downward acceleration
    f32 eyeHeight    = 1.6f;     // Eye position above feet (standing)
    f32 bodyRadius   = 0.3f;     // Half-width of character AABB (XZ)
    f32 bodyHeight   = 1.8f;     // Full height of character AABB (standing)

    // Crouch
    f32 crouchHeight          = 1.0f;   // AABB height when crouching
    f32 crouchEyeHeight       = 0.85f;  // Eye position above feet (crouching)
    f32 crouchSpeedMult       = 0.45f;  // Speed multiplier when crouching
    f32 crouchTransitionSpeed = 10.0f;  // Eye height lerp speed (units/sec factor)
};

class FPSController {
public:
    /// Set initial position and config.
    void Init(const glm::vec3& spawnPosition, const FPSControllerConfig& config = {});

    /// Process input, apply gravity, resolve collision.
    /// Call once per frame. cameraYaw is needed to orient
    /// movement relative to the camera facing direction.
    void Update(
        const InputActions& actions,
        const CollisionWorld& collision,
        f32 cameraYaw,
        f32 deltaTime
    );

    /// World-space position of the character's eyes.
    [[nodiscard]] glm::vec3 EyePosition() const;

    /// Center of the character AABB (feet center + half height).
    [[nodiscard]] const glm::vec3& Position() const { return m_position; }

    /// Current velocity (for debug display).
    [[nodiscard]] const glm::vec3& Velocity() const { return m_velocity; }

    /// Is the character standing on something?
    [[nodiscard]] bool IsGrounded() const { return m_grounded; }

    /// Is the character crouching?
    [[nodiscard]] bool IsCrouching() const { return m_crouching; }

    /// Readable config for debug UI.
    [[nodiscard]] const FPSControllerConfig& Config() const { return m_config; }

private:
    FPSControllerConfig m_config;
    glm::vec3 m_position{0.0f};   // Center of character AABB
    glm::vec3 m_velocity{0.0f};
    bool m_grounded = false;
    bool m_crouching = false;
    f32  m_currentEyeHeight = 1.6f; // Smoothly interpolated eye height
};

} // namespace freak
