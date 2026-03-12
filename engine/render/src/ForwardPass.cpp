#include <freak/render/ForwardPass.h>
#include <freak/render/CameraData.h>
#include <freak/render/MeshCache.h>
#include <freak/scene/Components.h>
#include <freak/core/Assert.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <bgfx/bgfx.h>
#include <entt/entt.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>

namespace freak {

// ─────────────────────────────────────────────
// Shader loading helper
// bgfx shaders are pre-compiled binaries.
// We load them from disk at init time.
// ─────────────────────────────────────────────

static bgfx::ShaderHandle LoadShader(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        FREAK_LOG_ERROR("Render", "Failed to open shader: {}", path);
        return BGFX_INVALID_HANDLE;
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    // bgfx requires the memory to persist, so we use bgfx::copy
    const bgfx::Memory* mem = bgfx::alloc(static_cast<u32>(size) + 1);
    file.read(reinterpret_cast<char*>(mem->data), size);
    mem->data[mem->size - 1] = '\0'; // bgfx requires null terminator

    return bgfx::createShader(mem);
}

Status ForwardPass::Init() {
    FREAK_PROFILE_SCOPE;

    // Load pre-compiled shaders
    // Path assumes working directory is project root.
    // Shader compilation is done offline by the build script.
    bgfx::ShaderHandle vsh = LoadShader("shaders/compiled/vs_forward.bin");
    bgfx::ShaderHandle fsh = LoadShader("shaders/compiled/fs_forward.bin");

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh)) {
        FREAK_LOG_ERROR("Render", "Failed to load forward shaders");
        if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
        if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
        return std::unexpected(Error::InitFailed);
    }

    m_program = bgfx::createProgram(vsh, fsh, true); // true = destroy shaders when program is destroyed
    if (!bgfx::isValid(m_program)) {
        FREAK_LOG_ERROR("Render", "Failed to create forward shader program");
        return std::unexpected(Error::InitFailed);
    }

    // Create uniform handles
    m_uMaterialDiffuse  = bgfx::createUniform("u_materialDiffuse",  bgfx::UniformType::Vec4);
    m_uMaterialEmissive = bgfx::createUniform("u_materialEmissive", bgfx::UniformType::Vec4);
    m_uLightDir         = bgfx::createUniform("u_lightDir",         bgfx::UniformType::Vec4);
    m_uLightColor       = bgfx::createUniform("u_lightColor",       bgfx::UniformType::Vec4);
    m_uAmbientColor     = bgfx::createUniform("u_ambientColor",     bgfx::UniformType::Vec4);
    m_uFogColor         = bgfx::createUniform("u_fogColor",         bgfx::UniformType::Vec4);
    m_uFogParams        = bgfx::createUniform("u_fogParams",        bgfx::UniformType::Vec4);
    m_uCameraPos        = bgfx::createUniform("u_cameraPos",        bgfx::UniformType::Vec4);

    FREAK_LOG_INFO("Render", "ForwardPass initialized");
    return {};
}

void ForwardPass::Shutdown() {
    auto safeDestroy = [](auto& handle) {
        if (bgfx::isValid(handle)) {
            bgfx::destroy(handle);
            handle = BGFX_INVALID_HANDLE;
        }
    };

    safeDestroy(m_program);
    safeDestroy(m_uMaterialDiffuse);
    safeDestroy(m_uMaterialEmissive);
    safeDestroy(m_uLightDir);
    safeDestroy(m_uLightColor);
    safeDestroy(m_uAmbientColor);
    safeDestroy(m_uFogColor);
    safeDestroy(m_uFogParams);
    safeDestroy(m_uCameraPos);

    FREAK_LOG_INFO("Render", "ForwardPass shutdown");
}

void ForwardPass::Execute(
    const CameraData& camera,
    const entt::registry& registry,
    const MeshCache& meshCache,
    const SceneLighting& lighting
) {
    FREAK_PROFILE_SCOPE_N("ForwardPass::Execute");

    // ── Set view transform (once per frame) ──────
    bgfx::setViewTransform(0,
        glm::value_ptr(camera.view),
        glm::value_ptr(camera.proj)
    );

    // ── Set per-frame lighting uniforms ──────────
    // Light direction: shader expects direction TO the light (negate scene direction)
    glm::vec3 toLightDir = glm::normalize(-lighting.lightDir);
    glm::vec4 lightDirPacked{toLightDir, lighting.lightIntensity};
    glm::vec4 lightColorPacked{lighting.lightColor, 1.0f};
    glm::vec4 ambientPacked{lighting.ambientColor, 1.0f};
    glm::vec4 fogColorPacked{lighting.fogColor, 1.0f};
    glm::vec4 fogParamsPacked{lighting.fogDensity, lighting.fogStart, lighting.fogEnd, 0.0f};
    glm::vec4 cameraPosPacked{camera.position, 1.0f};

    bgfx::setUniform(m_uLightDir,     glm::value_ptr(lightDirPacked));
    bgfx::setUniform(m_uLightColor,   glm::value_ptr(lightColorPacked));
    bgfx::setUniform(m_uAmbientColor, glm::value_ptr(ambientPacked));
    bgfx::setUniform(m_uFogColor,     glm::value_ptr(fogColorPacked));
    bgfx::setUniform(m_uFogParams,    glm::value_ptr(fogParamsPacked));
    bgfx::setUniform(m_uCameraPos,    glm::value_ptr(cameraPosPacked));

    // ── Iterate entities and submit draw calls ───
    m_drawCallCount = 0;
    m_triangleCount = 0;

    auto view = registry.view<const TransformComponent, const MeshComponent, const MaterialComponent>();
    for (auto [entity, transform, mesh, material] : view.each()) {
        if (mesh.meshID == MeshID::Invalid) {
            continue;
        }

        const MeshData& meshData = meshCache.Get(mesh.meshID);
        if (!bgfx::isValid(meshData.vbh) || !bgfx::isValid(meshData.ibh)) {
            continue;
        }

        // ── Set per-draw uniforms ────────────────
        glm::vec4 diffusePacked{material.diffuse, material.roughness};
        glm::vec4 emissivePacked{material.emissive, 0.0f};
        bgfx::setUniform(m_uMaterialDiffuse,  glm::value_ptr(diffusePacked));
        bgfx::setUniform(m_uMaterialEmissive, glm::value_ptr(emissivePacked));

        // ── Set model transform ──────────────────
        bgfx::setTransform(glm::value_ptr(transform.worldMatrix));

        // ── Set mesh buffers ─────────────────────
        bgfx::setVertexBuffer(0, meshData.vbh);
        bgfx::setIndexBuffer(meshData.ibh);

        // ── Render state ─────────────────────────
        u64 state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW      // Clockwise culling (bgfx default winding)
            | BGFX_STATE_MSAA;

        bgfx::setState(state);

        // ── Submit ───────────────────────────────
        bgfx::submit(0, m_program);

        m_drawCallCount++;
        m_triangleCount += meshData.indexCount / 3;
    }

    FREAK_PROFILE_PLOT("Draw Calls", static_cast<f64>(m_drawCallCount));
    FREAK_PROFILE_PLOT("Triangles",  static_cast<f64>(m_triangleCount));
}

} // namespace freak
