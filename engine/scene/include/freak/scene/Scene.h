#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Scene
// Owns the EnTT registry and manages entity lifetime.
// This is the data backbone — components live here,
// systems query here, but the scene itself has no
// knowledge of rendering, audio, or physics.
// Implementation: M1
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <entt/entt.hpp>

namespace freak {

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    [[nodiscard]] entt::registry& Registry() { return m_registry; }
    [[nodiscard]] const entt::registry& Registry() const { return m_registry; }

private:
    entt::registry m_registry;
};

} // namespace freak
