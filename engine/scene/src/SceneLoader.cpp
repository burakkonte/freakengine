#include <freak/scene/SceneLoader.h>
#include <freak/scene/Components.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstring>
#include <fstream>

namespace freak {

using json = nlohmann::json;

// ─────────────────────────────────────────────
// JSON helpers — read vec3 from [x, y, z] array
// ─────────────────────────────────────────────

static glm::vec3 ReadVec3(const json& j, const glm::vec3& fallback = {0.0f, 0.0f, 0.0f}) {
    if (!j.is_array() || j.size() < 3) return fallback;
    return {j[0].get<f32>(), j[1].get<f32>(), j[2].get<f32>()};
}

static glm::quat EulerDegreesToQuat(const glm::vec3& eulerDeg) {
    return glm::quat(glm::radians(eulerDeg));
}

// ─────────────────────────────────────────────
// Parse lighting/fog block
// ─────────────────────────────────────────────

static SceneLighting ParseLighting(const json& root) {
    SceneLighting lighting;

    if (root.contains("sun")) {
        auto& sun = root["sun"];
        if (sun.contains("direction"))  lighting.lightDir       = ReadVec3(sun["direction"], lighting.lightDir);
        if (sun.contains("color"))      lighting.lightColor     = ReadVec3(sun["color"], lighting.lightColor);
        if (sun.contains("intensity"))  lighting.lightIntensity = sun["intensity"].get<f32>();
        if (sun.contains("ambient"))    lighting.ambientColor   = ReadVec3(sun["ambient"], lighting.ambientColor);
    }

    if (root.contains("fog")) {
        auto& fog = root["fog"];
        if (fog.contains("color"))   lighting.fogColor   = ReadVec3(fog["color"], lighting.fogColor);
        if (fog.contains("density")) lighting.fogDensity = fog["density"].get<f32>();
        if (fog.contains("start"))   lighting.fogStart   = fog["start"].get<f32>();
        if (fog.contains("end"))     lighting.fogEnd     = fog["end"].get<f32>();
    }

    return lighting;
}

// ─────────────────────────────────────────────
// Parse and create entities
// ─────────────────────────────────────────────

static u32 ParseEntities(
    const json& root,
    entt::registry& registry,
    const MeshResolver& resolveMesh
) {
    if (!root.contains("entities") || !root["entities"].is_array()) {
        return 0;
    }

    u32 count = 0;
    for (auto& ent : root["entities"]) {
        // ── Mesh (required) ──
        if (!ent.contains("mesh") || !ent["mesh"].is_string()) {
            FREAK_LOG_WARN("Scene", "Entity missing 'mesh' field, skipping");
            continue;
        }

        std::string meshName = ent["mesh"].get<std::string>();
        MeshID meshID = resolveMesh(meshName);
        if (meshID == MeshID::Invalid) {
            FREAK_LOG_WARN("Scene", "Unknown mesh '{}', skipping entity", meshName);
            continue;
        }

        // ── Create entity ──
        auto entity = registry.create();

        // ── Transform ──
        TransformComponent transform;
        if (ent.contains("position")) {
            transform.position = ReadVec3(ent["position"]);
        }
        if (ent.contains("rotation")) {
            glm::vec3 eulerDeg = ReadVec3(ent["rotation"]);
            transform.rotation = EulerDegreesToQuat(eulerDeg);
        }
        if (ent.contains("scale")) {
            auto& s = ent["scale"];
            if (s.is_array()) {
                transform.scale = ReadVec3(s, {1.0f, 1.0f, 1.0f});
            } else if (s.is_number()) {
                f32 us = s.get<f32>();
                transform.scale = {us, us, us};
            }
        }
        RebuildWorldMatrix(transform);
        registry.emplace<TransformComponent>(entity, transform);

        // ── Mesh component ──
        registry.emplace<MeshComponent>(entity, MeshComponent{meshID});

        // ── Material ──
        MaterialComponent material;
        if (ent.contains("material")) {
            auto& mat = ent["material"];
            if (mat.contains("diffuse"))   material.diffuse   = ReadVec3(mat["diffuse"], material.diffuse);
            if (mat.contains("emissive"))  material.emissive  = ReadVec3(mat["emissive"], material.emissive);
            if (mat.contains("roughness")) material.roughness = mat["roughness"].get<f32>();
        }
        registry.emplace<MaterialComponent>(entity, material);

        // ── Name (optional, for debug) ──
        if (ent.contains("name") && ent["name"].is_string()) {
            NameComponent name;
            std::string n = ent["name"].get<std::string>();
            usize len = n.size() < 63 ? n.size() : 63;
            std::memcpy(name.name, n.c_str(), len);
            name.name[len] = '\0';
            registry.emplace<NameComponent>(entity, name);
        }

        // ── Collider (optional, M2A) ──
        // "collider": [hx, hy, hz]  — half-extents for AABB
        if (ent.contains("collider")) {
            ColliderComponent collider;
            collider.halfExtents = ReadVec3(ent["collider"], collider.halfExtents);
            registry.emplace<ColliderComponent>(entity, collider);
        }

        // ── Interactable (optional, M2A) ──
        // "interactable": "Examine"  — label shown on crosshair hover
        if (ent.contains("interactable") && ent["interactable"].is_string()) {
            InteractableComponent interact;
            std::string lbl = ent["interactable"].get<std::string>();
            usize len = lbl.size() < 63 ? lbl.size() : 63;
            std::memcpy(interact.label, lbl.c_str(), len);
            interact.label[len] = '\0';
            registry.emplace<InteractableComponent>(entity, interact);
        }

        count++;
    }

    return count;
}

// ─────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────

Result<SceneLoadResult> LoadScene(
    StringView path,
    entt::registry& registry,
    const MeshResolver& meshResolver
) {
    FREAK_PROFILE_SCOPE;

    FREAK_LOG_INFO("Scene", "Loading scene: {}", path);

    std::ifstream file{std::string{path}};
    if (!file.is_open()) {
        FREAK_LOG_ERROR("Scene", "Failed to open scene file: {}", path);
        return std::unexpected(Error::FileNotFound);
    }

    json root;
    try {
        root = json::parse(file);
    } catch (const json::parse_error& e) {
        FREAK_LOG_ERROR("Scene", "JSON parse error: {}", e.what());
        return std::unexpected(Error::InvalidData);
    }

    SceneLoadResult result;
    result.lighting = ParseLighting(root);
    result.entityCount = ParseEntities(root, registry, meshResolver);

    if (root.contains("name") && root["name"].is_string()) {
        FREAK_LOG_INFO("Scene", "Scene '{}' loaded: {} entities",
            root["name"].get<std::string>(), result.entityCount);
    } else {
        FREAK_LOG_INFO("Scene", "Scene loaded: {} entities", result.entityCount);
    }

    return result;
}

} // namespace freak
