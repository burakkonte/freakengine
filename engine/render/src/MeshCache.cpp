#include <freak/render/MeshCache.h>
#include <freak/render/Vertex.h>
#include <freak/core/Assert.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <cstring>
#include <vector>
#include <glm/vec3.hpp>

namespace freak {

Status MeshCache::Init() {
    FREAK_PROFILE_SCOPE;

    Vertex::Init();

    CreatePlane();
    CreateCube();
    CreateCylinder();

    FREAK_LOG_INFO("Render", "MeshCache initialized: {} built-in meshes", m_count);
    return {};
}

void MeshCache::Shutdown() {
    for (u32 i = 0; i < m_count; ++i) {
        auto& d = m_entries[i].data;
        if (bgfx::isValid(d.vbh)) bgfx::destroy(d.vbh);
        if (bgfx::isValid(d.ibh)) bgfx::destroy(d.ibh);
        d.vbh = BGFX_INVALID_HANDLE;
        d.ibh = BGFX_INVALID_HANDLE;
    }
    m_count = 0;
    FREAK_LOG_INFO("Render", "MeshCache shutdown");
}

MeshID MeshCache::FindByName(StringView name) const {
    for (u32 i = 0; i < m_count; ++i) {
        if (name == m_entries[i].name) {
            return static_cast<MeshID>(i);
        }
    }
    FREAK_LOG_WARN("Render", "MeshCache: unknown mesh '{}'", name);
    return MeshID::Invalid;
}

const MeshData& MeshCache::Get(MeshID id) const {
    u16 index = static_cast<u16>(id);
    FREAK_ASSERT(index < m_count);
    return m_entries[index].data;
}

void MeshCache::Register(StringView name, const MeshData& data) {
    FREAK_ASSERT(m_count < MaxMeshes);
    auto& entry = m_entries[m_count];
    usize len = name.size() < 31 ? name.size() : 31;
    std::memcpy(entry.name, name.data(), len);
    entry.name[len] = '\0';
    entry.data = data;
    m_count++;
}

// ─────────────────────────────────────────────
// Procedural mesh generators
// ─────────────────────────────────────────────

void MeshCache::CreatePlane() {
    // XZ plane, centered at origin, 1x1 (scaled by transform)
    const u32 white = PackColor(1.0f, 1.0f, 1.0f);
    const glm::vec3 up{0.0f, 1.0f, 0.0f};

    Vertex vertices[] = {
        {{-0.5f, 0.0f, -0.5f}, up, white},
        {{ 0.5f, 0.0f, -0.5f}, up, white},
        {{ 0.5f, 0.0f,  0.5f}, up, white},
        {{-0.5f, 0.0f,  0.5f}, up, white},
    };

    u16 indices[] = {
        0, 2, 1,
        0, 3, 2,
    };

    // bgfx::copy for stack-allocated data (bgfx takes ownership of the copy)
    MeshData data;
    data.vbh = bgfx::createVertexBuffer(
        bgfx::copy(vertices, sizeof(vertices)),
        Vertex::s_layout
    );
    data.ibh = bgfx::createIndexBuffer(
        bgfx::copy(indices, sizeof(indices))
    );
    data.indexCount = 6;

    Register("plane", data);
}

void MeshCache::CreateCube() {
    const u32 white = PackColor(1.0f, 1.0f, 1.0f);

    // 24 vertices (4 per face for correct normals)
    Vertex vertices[] = {
        // Front face (Z+)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, white},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, white},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, white},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, white},
        // Back face (Z-)
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, white},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, white},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, white},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, white},
        // Top face (Y+)
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, white},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, white},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, white},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, white},
        // Bottom face (Y-)
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, white},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, white},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, white},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, white},
        // Right face (X+)
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, white},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, white},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, white},
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, white},
        // Left face (X-)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, white},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, white},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, white},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, white},
    };

    u16 indices[] = {
         0,  1,  2,   0,  2,  3, // front
         4,  5,  6,   4,  6,  7, // back
         8,  9, 10,   8, 10, 11, // top
        12, 13, 14,  12, 14, 15, // bottom
        16, 17, 18,  16, 18, 19, // right
        20, 21, 22,  20, 22, 23, // left
    };

    MeshData data;
    data.vbh = bgfx::createVertexBuffer(
        bgfx::copy(vertices, sizeof(vertices)),
        Vertex::s_layout
    );
    data.ibh = bgfx::createIndexBuffer(
        bgfx::copy(indices, sizeof(indices))
    );
    data.indexCount = 36;

    Register("cube", data);
}

void MeshCache::CreateCylinder() {
    // Simple cylinder along Y axis, 16 segments
    constexpr u32 segments = 16;
    constexpr f32 radius = 0.5f;
    constexpr f32 halfHeight = 0.5f;
    const u32 white = PackColor(1.0f, 1.0f, 1.0f);

    std::vector<Vertex> vertices;
    std::vector<u16> indices;
    vertices.reserve((segments + 1) * 2 + (segments + 1) * 2);

    // Side vertices
    for (u32 i = 0; i <= segments; ++i) {
        f32 angle = (static_cast<f32>(i) / static_cast<f32>(segments)) * 6.28318530718f;
        f32 cx = std::cos(angle);
        f32 cz = std::sin(angle);
        glm::vec3 normal{cx, 0.0f, cz};

        vertices.push_back({{cx * radius, -halfHeight, cz * radius}, normal, white});
        vertices.push_back({{cx * radius,  halfHeight, cz * radius}, normal, white});
    }

    // Side indices
    for (u32 i = 0; i < segments; ++i) {
        u16 bl = static_cast<u16>(i * 2);
        u16 br = static_cast<u16>((i + 1) * 2);
        u16 tl = static_cast<u16>(bl + 1);
        u16 tr = static_cast<u16>(br + 1);
        indices.push_back(bl); indices.push_back(br); indices.push_back(tr);
        indices.push_back(bl); indices.push_back(tr); indices.push_back(tl);
    }

    // Cap centers
    u16 topCenter = static_cast<u16>(vertices.size());
    vertices.push_back({{0.0f,  halfHeight, 0.0f}, {0.0f,  1.0f, 0.0f}, white});
    u16 botCenter = static_cast<u16>(vertices.size());
    vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.0f, -1.0f, 0.0f}, white});

    // Cap rim vertices (separate for correct normals)
    u16 topRimStart = static_cast<u16>(vertices.size());
    for (u32 i = 0; i <= segments; ++i) {
        f32 angle = (static_cast<f32>(i) / static_cast<f32>(segments)) * 6.28318530718f;
        f32 cx = std::cos(angle);
        f32 cz = std::sin(angle);
        vertices.push_back({{cx * radius,  halfHeight, cz * radius}, {0.0f,  1.0f, 0.0f}, white});
    }
    u16 botRimStart = static_cast<u16>(vertices.size());
    for (u32 i = 0; i <= segments; ++i) {
        f32 angle = (static_cast<f32>(i) / static_cast<f32>(segments)) * 6.28318530718f;
        f32 cx = std::cos(angle);
        f32 cz = std::sin(angle);
        vertices.push_back({{cx * radius, -halfHeight, cz * radius}, {0.0f, -1.0f, 0.0f}, white});
    }

    // Cap indices
    for (u32 i = 0; i < segments; ++i) {
        // Top cap (winding: center, i+1, i for correct normal direction)
        indices.push_back(topCenter);
        indices.push_back(static_cast<u16>(topRimStart + i + 1));
        indices.push_back(static_cast<u16>(topRimStart + i));
        // Bottom cap
        indices.push_back(botCenter);
        indices.push_back(static_cast<u16>(botRimStart + i));
        indices.push_back(static_cast<u16>(botRimStart + i + 1));
    }

    MeshData data;
    data.vbh = bgfx::createVertexBuffer(
        bgfx::copy(vertices.data(), static_cast<u32>(vertices.size() * sizeof(Vertex))),
        Vertex::s_layout
    );
    data.ibh = bgfx::createIndexBuffer(
        bgfx::copy(indices.data(), static_cast<u32>(indices.size() * sizeof(u16)))
    );
    data.indexCount = static_cast<u32>(indices.size());

    Register("cylinder", data);
}

// Static vertex layout definition
bgfx::VertexLayout Vertex::s_layout;

} // namespace freak
