#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Render Device
// Initializes and owns the bgfx rendering context.
// This is the lowest layer of the renderer — it handles
// init, shutdown, frame submission, and resize.
// Higher-level rendering (meshes, lights, post-process)
// will be built on top in later milestones.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

namespace freak {

class Window;

struct RenderConfig {
    // bgfx renderer type — 0 means auto-detect (prefers D3D11 on Windows)
    i32 rendererType = 0;

    // Initial resolution
    i32 width  = 1280;
    i32 height = 720;

    // Enable vsync
    bool vsync = true;

    // Debug/profile flags for bgfx
    bool debugStats = false;
    bool debugText  = false;
};

class RenderDevice {
public:
    RenderDevice() = default;
    ~RenderDevice();

    // Initialize bgfx with the given window and config.
    [[nodiscard]] Status Init(const Window& window, const RenderConfig& config);

    // Shutdown bgfx.
    void Shutdown();

    // Call when the window is resized.
    void OnResize(i32 width, i32 height);

    // Begin a new frame — clears the backbuffer.
    void BeginFrame();

    // End frame — submits to bgfx and presents.
    void EndFrame();

    // Toggle debug stats overlay (bgfx built-in)
    void ToggleStats();

    [[nodiscard]] i32 Width() const { return m_width; }
    [[nodiscard]] i32 Height() const { return m_height; }
    [[nodiscard]] bool IsInitialized() const { return m_initialized; }

private:
    i32 m_width  = 0;
    i32 m_height = 0;
    bool m_initialized = false;
    bool m_showStats   = false;
    u32 m_resetFlags   = 0;
};

} // namespace freak
