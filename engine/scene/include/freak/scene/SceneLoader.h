#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Scene Loader
// Reads a JSON scene description and populates
// an EnTT registry with entities and components.
// Also extracts lighting and fog parameters.
//
// This module knows about JSON, Components, and SceneLighting.
// It does NOT know about bgfx, shaders, or rendering.
// Mesh name resolution is injected via a callback.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/scene/MeshTypes.h>
#include <freak/scene/SceneLighting.h>

#include <entt/entt.hpp>
#include <functional>

namespace freak {

// Callback to resolve a mesh name to a MeshID.
// Injected by the caller so Scene never depends on Render.
using MeshResolver = std::function<MeshID(StringView)>;

struct SceneLoadResult {
    SceneLighting lighting;
    u32 entityCount = 0;
};

// Load a scene file and populate the registry.
// meshResolver is called at load time to convert mesh names to IDs.
[[nodiscard]] Result<SceneLoadResult> LoadScene(
    StringView path,
    entt::registry& registry,
    const MeshResolver& meshResolver
);

} // namespace freak
