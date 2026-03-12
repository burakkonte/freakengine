#include <freak/physics/CollisionWorld.h>
#include <freak/core/Assert.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace freak {

// ─────────────────────────────────────────────
// AABB overlap test
// ─────────────────────────────────────────────

static bool Overlaps(const AABB& a, const AABB& b) {
    glm::vec3 aMin = a.Min();
    glm::vec3 aMax = a.Max();
    glm::vec3 bMin = b.Min();
    glm::vec3 bMax = b.Max();

    return (aMin.x < bMax.x && aMax.x > bMin.x)
        && (aMin.y < bMax.y && aMax.y > bMin.y)
        && (aMin.z < bMax.z && aMax.z > bMin.z);
}

// ─────────────────────────────────────────────
// Penetration depth along each axis
// ─────────────────────────────────────────────

static glm::vec3 PenetrationDepth(const AABB& a, const AABB& b) {
    // Positive = overlapping on that axis, negative = gap
    f32 dx = (a.halfExtents.x + b.halfExtents.x) - std::abs(a.center.x - b.center.x);
    f32 dy = (a.halfExtents.y + b.halfExtents.y) - std::abs(a.center.y - b.center.y);
    f32 dz = (a.halfExtents.z + b.halfExtents.z) - std::abs(a.center.z - b.center.z);
    return {dx, dy, dz};
}

// ─────────────────────────────────────────────
// Ray-AABB intersection (slab method)
// ─────────────────────────────────────────────

static bool RayAABB(
    const glm::vec3& origin,
    const glm::vec3& invDir,
    const AABB& box,
    f32 maxDist,
    f32& outT
) {
    glm::vec3 bMin = box.Min();
    glm::vec3 bMax = box.Max();

    f32 t1 = (bMin.x - origin.x) * invDir.x;
    f32 t2 = (bMax.x - origin.x) * invDir.x;
    f32 t3 = (bMin.y - origin.y) * invDir.y;
    f32 t4 = (bMax.y - origin.y) * invDir.y;
    f32 t5 = (bMin.z - origin.z) * invDir.z;
    f32 t6 = (bMax.z - origin.z) * invDir.z;

    f32 tMin = std::max({std::min(t1, t2), std::min(t3, t4), std::min(t5, t6)});
    f32 tMax = std::min({std::max(t1, t2), std::max(t3, t4), std::max(t5, t6)});

    if (tMax < 0.0f || tMin > tMax || tMin > maxDist) {
        return false;
    }

    outT = (tMin >= 0.0f) ? tMin : tMax;
    return outT <= maxDist;
}

// ─────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────

Status CollisionWorld::Init() {
    m_bodies.clear();
    FREAK_LOG_INFO("Collision", "CollisionWorld initialized (M2A AABB mode)");
    return {};
}

void CollisionWorld::Shutdown() {
    m_bodies.clear();
    FREAK_LOG_INFO("Collision", "CollisionWorld shutdown");
}

u32 CollisionWorld::AddStaticBox(const glm::vec3& center, const glm::vec3& halfExtents) {
    u32 index = static_cast<u32>(m_bodies.size());
    m_bodies.push_back({center, halfExtents});
    return index;
}

MoveResult CollisionWorld::MoveAndSlide(
    const AABB& body,
    const glm::vec3& displacement
) const {
    FREAK_PROFILE_SCOPE_N("CollisionWorld::MoveAndSlide");

    MoveResult result;
    result.position = body.center;
    result.grounded = false;

    // ── Per-axis sweep and resolve ──
    // Move along each axis independently, check for
    // overlaps, and push out along the movement axis.
    // This gives clean sliding along walls.

    // X axis
    {
        AABB swept = body;
        swept.center.x = result.position.x + displacement.x;
        swept.center.y = result.position.y;
        swept.center.z = result.position.z;

        for (const AABB& wall : m_bodies) {
            if (Overlaps(swept, wall)) {
                glm::vec3 pen = PenetrationDepth(swept, wall);
                if (pen.x > 0.0f) {
                    f32 sign = (swept.center.x > wall.center.x) ? 1.0f : -1.0f;
                    swept.center.x += sign * pen.x;
                }
            }
        }
        result.position.x = swept.center.x;
    }

    // Y axis
    {
        AABB swept = body;
        swept.center.x = result.position.x;
        swept.center.y = result.position.y + displacement.y;
        swept.center.z = result.position.z;

        for (const AABB& wall : m_bodies) {
            if (Overlaps(swept, wall)) {
                glm::vec3 pen = PenetrationDepth(swept, wall);
                if (pen.y > 0.0f) {
                    f32 sign = (swept.center.y > wall.center.y) ? 1.0f : -1.0f;
                    swept.center.y += sign * pen.y;

                    // Grounded if pushed upward (standing on something)
                    if (sign > 0.0f) {
                        result.grounded = true;
                    }
                }
            }
        }
        result.position.y = swept.center.y;
    }

    // Z axis
    {
        AABB swept = body;
        swept.center.x = result.position.x;
        swept.center.y = result.position.y;
        swept.center.z = result.position.z + displacement.z;

        for (const AABB& wall : m_bodies) {
            if (Overlaps(swept, wall)) {
                glm::vec3 pen = PenetrationDepth(swept, wall);
                if (pen.z > 0.0f) {
                    f32 sign = (swept.center.z > wall.center.z) ? 1.0f : -1.0f;
                    swept.center.z += sign * pen.z;
                }
            }
        }
        result.position.z = swept.center.z;
    }

    // ── Ground plane (Y=0) ──
    f32 feetY = result.position.y - body.halfExtents.y;
    if (feetY < 0.0f) {
        result.position.y = body.halfExtents.y;
        result.grounded = true;
    }

    return result;
}

bool CollisionWorld::TestOverlap(const AABB& box) const {
    for (const AABB& body : m_bodies) {
        if (Overlaps(box, body)) {
            return true;
        }
    }
    return false;
}

RayHit CollisionWorld::Raycast(
    const glm::vec3& origin,
    const glm::vec3& direction,
    f32 maxDistance
) const {
    FREAK_PROFILE_SCOPE_N("CollisionWorld::Raycast");

    RayHit closest;
    closest.distance = maxDistance;

    // Precompute inverse direction (avoid per-box division)
    constexpr f32 eps = 1e-8f;
    glm::vec3 invDir{
        1.0f / (std::abs(direction.x) > eps ? direction.x : eps),
        1.0f / (std::abs(direction.y) > eps ? direction.y : eps),
        1.0f / (std::abs(direction.z) > eps ? direction.z : eps)
    };

    for (u32 i = 0; i < static_cast<u32>(m_bodies.size()); ++i) {
        f32 t = 0.0f;
        if (RayAABB(origin, invDir, m_bodies[i], closest.distance, t)) {
            if (t < closest.distance) {
                closest.hit = true;
                closest.distance = t;
                closest.point = origin + direction * t;
                closest.bodyIndex = i;

                // Approximate hit normal from the hit point relative to box center
                glm::vec3 localHit = closest.point - m_bodies[i].center;
                glm::vec3 d = localHit / m_bodies[i].halfExtents;
                // The axis with the largest absolute value is the face we hit
                if (std::abs(d.x) > std::abs(d.y) && std::abs(d.x) > std::abs(d.z)) {
                    closest.normal = {(d.x > 0.0f ? 1.0f : -1.0f), 0.0f, 0.0f};
                } else if (std::abs(d.y) > std::abs(d.z)) {
                    closest.normal = {0.0f, (d.y > 0.0f ? 1.0f : -1.0f), 0.0f};
                } else {
                    closest.normal = {0.0f, 0.0f, (d.z > 0.0f ? 1.0f : -1.0f)};
                }
            }
        }
    }

    return closest;
}

} // namespace freak
