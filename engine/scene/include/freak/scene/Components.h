#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Scene Components
// Plain data structs attached to entities via EnTT.
// These are the contract between Scene and Render.
//
// Rules:
// - Components are data only. No methods, no logic.
// - Scene module defines them. Render module reads them.
// - No bgfx types here — MeshID is engine-defined.
// - No hierarchy in M1. Every transform is world-space.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/scene/MeshTypes.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

namespace freak {

// ── Transform ────────────────────────────────
// World-space transform. No parent, no hierarchy.
// The worldMatrix is rebuilt from pos/rot/scale
// whenever the scene is loaded or modified.
struct TransformComponent {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; // Identity
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::mat4 worldMatrix{1.0f}; // Cached TRS matrix
};

// Rebuild the cached worldMatrix from position/rotation/scale.
void RebuildWorldMatrix(TransformComponent& t);

// ── Mesh reference ───────────────────────────
// Points to a mesh in the MeshCache by ID (resolved at load time).
struct MeshComponent {
    MeshID meshID = MeshID::Invalid;
};

// ── Material ─────────────────────────────────
// Flat color material. No textures in M1.
struct MaterialComponent {
    glm::vec3 diffuse{0.5f, 0.5f, 0.5f};
    glm::vec3 emissive{0.0f, 0.0f, 0.0f};
    f32 roughness = 0.8f;
};

// ── Name tag (optional, debug only) ──────────
struct NameComponent {
    char name[64] = {};
};

// ── Collider (M2A) ─────────────────────────
// Marks an entity as a static collision body.
// The collision world reads these at scene load
// to build its AABB list. halfExtents defines
// the box size in local space.
struct ColliderComponent {
    glm::vec3 halfExtents{0.5f, 0.5f, 0.5f};
};

// ── Interactable (M2A) ─────────────────────
// Marks an entity as something the player can
// interact with via crosshair raycast.
struct InteractableComponent {
    char label[64] = {};  // e.g. "Examine", "Open"
};

} // namespace freak
