#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Collision World (M2A)
// Simple AABB-based collision for static world
// geometry. Provides move-and-slide resolution
// and raycasting against static bodies.
//
// This will be replaced by Jolt in M2B if the
// level design outgrows axis-aligned boxes.
//
// Public API is kept small and Jolt-compatible
// so the upgrade path is clean.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <glm/vec3.hpp>

#include <vector>

namespace freak {

/// Axis-aligned bounding box defined by center + half-extents.
struct AABB {
    glm::vec3 center{0.0f};
    glm::vec3 halfExtents{0.5f};

    [[nodiscard]] glm::vec3 Min() const { return center - halfExtents; }
    [[nodiscard]] glm::vec3 Max() const { return center + halfExtents; }
};

/// Result of a raycast against the collision world.
struct RayHit {
    bool hit       = false;
    f32 distance   = 0.0f;
    glm::vec3 point{0.0f};
    glm::vec3 normal{0.0f};
    u32 bodyIndex  = 0;   // index into the static body array
};

/// Result of move-and-slide collision resolution.
struct MoveResult {
    glm::vec3 position{0.0f};
    bool grounded = false;
};

class CollisionWorld {
public:
    /// Clear all bodies and prepare for use.
    Status Init();

    /// Release resources.
    void Shutdown();

    /// Add a static AABB body. Returns the body index.
    u32 AddStaticBox(const glm::vec3& center, const glm::vec3& halfExtents);

    /// Move an AABB through the world with per-axis sweep-and-slide.
    /// Returns the resolved position and whether the body is grounded.
    [[nodiscard]] MoveResult MoveAndSlide(
        const AABB& body,
        const glm::vec3& displacement
    ) const;

    /// Cast a ray and return the first hit.
    [[nodiscard]] RayHit Raycast(
        const glm::vec3& origin,
        const glm::vec3& direction,
        f32 maxDistance
    ) const;

    /// Test if an AABB overlaps any static body (used for stand-up checks).
    [[nodiscard]] bool TestOverlap(const AABB& box) const;

    /// Number of static bodies currently registered.
    [[nodiscard]] u32 BodyCount() const { return static_cast<u32>(m_bodies.size()); }

    /// Access a static body by index (for debug draw).
    [[nodiscard]] const AABB& GetBody(u32 index) const { return m_bodies[index]; }

private:
    std::vector<AABB> m_bodies;
};

} // namespace freak
