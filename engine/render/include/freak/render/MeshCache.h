#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Mesh Cache
// Owns GPU vertex/index buffers for procedural meshes.
// In M1 this generates cube and plane geometry.
// Later milestones will add loaded meshes from the
// asset baker, but the lookup-by-ID interface stays.
//
// Design: Scene loading resolves mesh names to MeshID
// at load time. The hot render path never touches strings.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <freak/scene/MeshTypes.h>
#include <bgfx/bgfx.h>
#include <string_view>

namespace freak {

struct MeshData {
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  ibh = BGFX_INVALID_HANDLE;
    u32 indexCount = 0;
};

class MeshCache {
public:
    // Initialize and generate built-in procedural meshes.
    Status Init();

    // Destroy all GPU buffers.
    void Shutdown();

    // Resolve a mesh name to a MeshID. Returns MeshID::Invalid if not found.
    // Call at load time (scene loading), NOT in the render loop.
    [[nodiscard]] MeshID FindByName(StringView name) const;

    // Get mesh data for rendering. MeshID must be valid.
    [[nodiscard]] const MeshData& Get(MeshID id) const;

    // Number of loaded meshes.
    [[nodiscard]] u32 Count() const { return m_count; }

private:
    void CreateCube();
    void CreatePlane();
    void CreateCylinder();

    void Register(StringView name, const MeshData& data);

    // Fixed capacity — we won't have hundreds of mesh types in M1.
    static constexpr u32 MaxMeshes = 32;

    struct Entry {
        char name[32] = {};
        MeshData data;
    };

    Entry m_entries[MaxMeshes] = {};
    u32 m_count = 0;
};

} // namespace freak
