#include <freak/render/Camera.h>
#include <freak/render/CameraData.h>
#include <freak/platform/Input.h>
#include <freak/core/Profiler.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <cmath>

namespace freak {

void DebugCamera::Init(const CameraState& state) {
    m_state = state;
    UpdateVectors();
    UpdateViewMatrix();
    UpdateProjMatrix();
}

void DebugCamera::Update(const Input& input, f64 deltaTime) {
    FREAK_PROFILE_SCOPE_N("DebugCamera::Update");

    f32 dt = static_cast<f32>(deltaTime);

    // ── Mouse look ───────────────────────────
    if (input.IsMouseCaptured()) {
        f32 dx = input.MouseDeltaX() * m_sensitivity;
        f32 dy = input.MouseDeltaY() * m_sensitivity;

        m_state.yaw   -= dx;  // LH: negate for correct screen-space rotation
        m_state.pitch -= dy;  // Mouse up = look up

        // Clamp pitch to prevent gimbal flip
        m_state.pitch = std::clamp(m_state.pitch, -89.0f, 89.0f);

        // Wrap yaw to keep it sane
        if (m_state.yaw > 360.0f) m_state.yaw -= 360.0f;
        if (m_state.yaw < -360.0f) m_state.yaw += 360.0f;

        UpdateVectors();
    }

    // ── Speed control via scroll (when captured) ──
    // Scroll wheel adjusts base speed multiplicatively
    // (Not yet wired — Input doesn't expose scroll in M0.
    //  Will add in the next Input update. For now, use
    //  +/- keys as a fallback.)
    if (input.IsKeyDown(SDL_SCANCODE_EQUALS)) {
        m_baseSpeed *= 1.0f + 2.0f * dt;
    }
    if (input.IsKeyDown(SDL_SCANCODE_MINUS)) {
        m_baseSpeed *= 1.0f - 2.0f * dt;
    }
    m_baseSpeed = std::clamp(m_baseSpeed, 0.5f, 200.0f);

    // ── Movement ─────────────────────────────
    glm::vec3 inputDir{0.0f};

    if (input.IsKeyDown(SDL_SCANCODE_W)) inputDir += m_forward;
    if (input.IsKeyDown(SDL_SCANCODE_S)) inputDir -= m_forward;
    if (input.IsKeyDown(SDL_SCANCODE_D)) inputDir += m_right;
    if (input.IsKeyDown(SDL_SCANCODE_A)) inputDir -= m_right;
    if (input.IsKeyDown(SDL_SCANCODE_E) || input.IsKeyDown(SDL_SCANCODE_SPACE)) {
        inputDir.y += 1.0f;
    }
    if (input.IsKeyDown(SDL_SCANCODE_Q) || input.IsKeyDown(SDL_SCANCODE_LCTRL)) {
        inputDir.y -= 1.0f;
    }

    // Normalize input direction (prevent faster diagonal movement)
    f32 inputLen = glm::length(inputDir);
    if (inputLen > 0.001f) {
        inputDir /= inputLen;
    }

    // Apply speed + sprint
    f32 speed = m_baseSpeed;
    if (input.IsKeyDown(SDL_SCANCODE_LSHIFT)) {
        speed *= m_sprintMult;
    }

    glm::vec3 targetVelocity = inputDir * speed;

    // Smooth acceleration/deceleration — prevents jerky movement
    f32 blend = 1.0f - std::exp(-m_smoothFactor * dt);
    m_velocity = glm::mix(m_velocity, targetVelocity, blend);

    // Integrate position
    m_state.position += m_velocity * dt;

    UpdateViewMatrix();
}

void DebugCamera::SetAspect(f32 aspect) {
    m_aspect = aspect;
    UpdateProjMatrix();
}

void DebugCamera::SetFOV(f32 fovDegrees) {
    m_fovDegrees = fovDegrees;
    UpdateProjMatrix();
}

void DebugCamera::SetClipPlanes(f32 near, f32 far) {
    m_nearClip = near;
    m_farClip = far;
    UpdateProjMatrix();
}

void DebugCamera::SetSensitivity(f32 sensitivity) {
    m_sensitivity = sensitivity;
}

void DebugCamera::UpdateVectors() {
    f32 yawRad   = glm::radians(m_state.yaw);
    f32 pitchRad = glm::radians(m_state.pitch);

    m_forward.x = std::cos(yawRad) * std::cos(pitchRad);
    m_forward.y = std::sin(pitchRad);
    m_forward.z = std::sin(yawRad) * std::cos(pitchRad);
    m_forward = glm::normalize(m_forward);

    // World up is always Y — LH cross product order
    constexpr glm::vec3 worldUp{0.0f, 1.0f, 0.0f};
    m_right = glm::normalize(glm::cross(worldUp, m_forward));
    m_up    = glm::normalize(glm::cross(m_forward, m_right));
}

void DebugCamera::UpdateViewMatrix() {
    m_view = glm::lookAtLH(m_state.position, m_state.position + m_forward, m_up);
}

void DebugCamera::UpdateProjMatrix() {
    // bgfx expects a specific projection matrix format.
    // glm::perspectiveLH_ZO gives left-handed, [0,1] depth — works for D3D.
    // For OpenGL backend, bgfx handles the conversion internally.
    m_proj = glm::perspectiveLH_ZO(
        glm::radians(m_fovDegrees),
        m_aspect,
        m_nearClip,
        m_farClip
    );
}

CameraData DebugCamera::GetCameraData() const {
    return { m_view, m_proj, m_state.position };
}

} // namespace freak
