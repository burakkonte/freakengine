#include <freak/framework/FPSController.h>
#include <freak/framework/InputActions.h>
#include <freak/physics/CollisionWorld.h>
#include <freak/core/Profiler.h>

#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>

#include <cmath>
#include <algorithm>

namespace freak {

void FPSController::Init(const glm::vec3& spawnPosition, const FPSControllerConfig& config) {
    m_config = config;

    // spawnPosition is feet position — convert to AABB center
    m_position = spawnPosition;
    m_position.y += m_config.bodyHeight * 0.5f;

    m_velocity = {0.0f, 0.0f, 0.0f};
    m_grounded = false;
    m_crouching = false;
    m_currentEyeHeight = m_config.eyeHeight;
}

void FPSController::Update(
    const InputActions& actions,
    const CollisionWorld& collision,
    f32 cameraYaw,
    f32 deltaTime
) {
    FREAK_PROFILE_SCOPE_N("FPSController::Update");

    // ── Crouch state ──
    bool wantsCrouch = actions.crouch;

    if (wantsCrouch && !m_crouching) {
        // Enter crouch: shrink AABB, adjust center downward
        f32 heightDelta = m_config.bodyHeight - m_config.crouchHeight;
        m_position.y -= heightDelta * 0.5f;
        m_crouching = true;
    } else if (!wantsCrouch && m_crouching && m_grounded) {
        // Try to stand up: check if there's room above
        f32 heightDelta = m_config.bodyHeight - m_config.crouchHeight;

        AABB standCheck;
        standCheck.center = m_position;
        standCheck.center.y += heightDelta * 0.5f;
        standCheck.halfExtents = {m_config.bodyRadius, m_config.bodyHeight * 0.5f, m_config.bodyRadius};

        if (!collision.TestOverlap(standCheck)) {
            m_position.y += heightDelta * 0.5f;
            m_crouching = false;
        }
        // else: can't stand, stay crouched
    }

    // Current body dimensions
    f32 currentHeight = m_crouching ? m_config.crouchHeight : m_config.bodyHeight;
    f32 targetEyeHeight = m_crouching ? m_config.crouchEyeHeight : m_config.eyeHeight;

    // Smooth eye height transition
    f32 blend = 1.0f - std::exp(-m_config.crouchTransitionSpeed * deltaTime);
    m_currentEyeHeight += (targetEyeHeight - m_currentEyeHeight) * blend;

    // ── Build movement direction from input + camera yaw ──
    // LH coordinate system: forward = (cos(yaw), 0, sin(yaw))
    // Right = cross(worldUp, forward) = (sin(yaw), 0, -cos(yaw))
    // Input: move.y > 0 = forward, move.x > 0 = right
    f32 yawRad = glm::radians(cameraYaw);
    glm::vec3 forward{std::cos(yawRad), 0.0f, std::sin(yawRad)};
    glm::vec3 right{std::sin(yawRad), 0.0f, -std::cos(yawRad)};

    glm::vec3 moveDir{0.0f};
    moveDir += forward * actions.move.y;
    moveDir += right * actions.move.x;

    // Normalize to prevent faster diagonal movement
    f32 moveLen = glm::length(moveDir);
    if (moveLen > 0.001f) {
        moveDir /= moveLen;
    }

    // Apply speed (crouch overrides sprint)
    f32 speed = m_config.moveSpeed;
    if (m_crouching) {
        speed *= m_config.crouchSpeedMult;
    } else if (actions.sprint) {
        speed *= m_config.sprintMult;
    }

    // Set horizontal velocity directly (no acceleration — snappy FPS feel)
    m_velocity.x = moveDir.x * speed;
    m_velocity.z = moveDir.z * speed;

    // ── Jump ──
    if (actions.jump && m_grounded) {
        m_velocity.y = m_config.jumpSpeed;
        m_grounded = false;

        // Uncrouch on jump if crouching
        if (m_crouching) {
            f32 heightDelta = m_config.bodyHeight - m_config.crouchHeight;
            m_position.y += heightDelta * 0.5f;
            m_crouching = false;
        }
    }

    // ── Gravity ──
    if (!m_grounded) {
        m_velocity.y -= m_config.gravity * deltaTime;
    }

    // ── Collision resolution ──
    AABB body;
    body.center = m_position;
    body.halfExtents = {m_config.bodyRadius, currentHeight * 0.5f, m_config.bodyRadius};

    glm::vec3 displacement = m_velocity * deltaTime;
    MoveResult result = collision.MoveAndSlide(body, displacement);

    // If grounded, kill downward velocity
    if (result.grounded && m_velocity.y < 0.0f) {
        m_velocity.y = 0.0f;
    }

    m_position = result.position;
    m_grounded = result.grounded;
}

glm::vec3 FPSController::EyePosition() const {
    // feet = center - halfHeight
    // eye = feet + currentEyeHeight
    f32 currentHeight = m_crouching ? m_config.crouchHeight : m_config.bodyHeight;
    f32 halfHeight = currentHeight * 0.5f;
    return {m_position.x, m_position.y - halfHeight + m_currentEyeHeight, m_position.z};
}

} // namespace freak
