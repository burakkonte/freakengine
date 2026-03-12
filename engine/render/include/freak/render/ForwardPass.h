#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Forward Render Pass
// Takes camera + scene registry, submits all
// renderable geometry to bgfx with lighting and fog.
// One pass, one shader, one view. Minimal and correct.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/render/CameraData.h>
#include <freak/scene/SceneLighting.h>
#include <bgfx/bgfx.h>
#include <entt/entt.hpp>

namespace freak {

class MeshCache;

class ForwardPass {
public:
    // Load shader program. Call once at startup (after bgfx init).
    [[nodiscard]] Status Init();

    // Destroy shader program.
    void Shutdown();

    // Render all entities with MeshComponent + MaterialComponent + TransformComponent.
    // camera: view/proj matrices and world position
    // registry: scene entities to render
    // meshCache: resolves MeshID to GPU buffers
    // lighting: scene-wide light and fog parameters
    void Execute(
        const CameraData& camera,
        const entt::registry& registry,
        const MeshCache& meshCache,
        const SceneLighting& lighting
    );

    // Stats from last frame
    [[nodiscard]] u32 DrawCallCount() const { return m_drawCallCount; }
    [[nodiscard]] u32 TriangleCount() const { return m_triangleCount; }

private:
    bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;

    // Uniform handles (created once, reused every frame)
    bgfx::UniformHandle m_uMaterialDiffuse  = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uMaterialEmissive = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uLightDir         = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uLightColor       = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uAmbientColor     = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uFogColor         = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uFogParams        = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle m_uCameraPos        = BGFX_INVALID_HANDLE;

    u32 m_drawCallCount = 0;
    u32 m_triangleCount = 0;
};

} // namespace freak
