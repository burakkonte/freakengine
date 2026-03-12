#include <freak/framework/GameplayCamera.h>
#include <freak/core/Profiler.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>

namespace freak {

void GameplayCamera::Init(f32 yaw, f32 pitch) {
    m_yaw = yaw;
    m_pitch = pitch;
    UpdateVectors();

    // Build initial projection
    m_proj = glm::perspectiveLH_ZO(
        glm::radians(m_fov), m_aspect, m_nearClip, m_farClip
    );
}

void GameplayCamera::Update(const glm::vec3& eyePosition, f32 lookDeltaX, f32 lookDeltaY) {
    FREAK_PROFILE_SCOPE_N("GameplayCamera::Update");

    m_eyePosition = eyePosition;

    // Apply mouse look
    m_yaw   -= lookDeltaX * m_sensitivity;  // LH: negate for correct screen-space rotation
    m_pitch -= lookDeltaY * m_sensitivity;  // Mouse up = look up

    // Clamp pitch
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    // Wrap yaw
    if (m_yaw > 360.0f) m_yaw -= 360.0f;
    if (m_yaw < -360.0f) m_yaw += 360.0f;

    UpdateVectors();

    // Rebuild view matrix
    m_view = glm::lookAtLH(m_eyePosition, m_eyePosition + m_forward, m_up);
}

void GameplayCamera::SetAspect(f32 aspect) {
    m_aspect = aspect;
    m_proj = glm::perspectiveLH_ZO(
        glm::radians(m_fov), m_aspect, m_nearClip, m_farClip
    );
}

CameraData GameplayCamera::GetCameraData() const {
    return {m_view, m_proj, m_eyePosition};
}

void GameplayCamera::UpdateVectors() {
    f32 yawRad   = glm::radians(m_yaw);
    f32 pitchRad = glm::radians(m_pitch);

    m_forward.x = std::cos(yawRad) * std::cos(pitchRad);
    m_forward.y = std::sin(pitchRad);
    m_forward.z = std::sin(yawRad) * std::cos(pitchRad);
    m_forward = glm::normalize(m_forward);

    // LH cross product order
    constexpr glm::vec3 worldUp{0.0f, 1.0f, 0.0f};
    m_right = glm::normalize(glm::cross(worldUp, m_forward));
    m_up    = glm::normalize(glm::cross(m_forward, m_right));
}

} // namespace freak
