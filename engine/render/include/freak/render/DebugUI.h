#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Debug UI (ImGui via bgfx)
// Provides the ImGui integration for in-game debug overlays.
// This is NOT the game HUD — it's for developer tools only.
// Stripped entirely from release builds.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

union SDL_Event;

namespace freak {

class RenderDevice;

#ifdef FREAK_IMGUI_ENABLED

namespace DebugUI {

    // Initialize ImGui with bgfx rendering backend.
    [[nodiscard]] Status Init(f32 fontSize = 16.0f);

    // Shutdown ImGui.
    void Shutdown();

    // Call at the start of each frame before any ImGui calls.
    void BeginFrame(i32 width, i32 height, f64 deltaTime);

    // Call at the end of the frame to render all ImGui draw data via bgfx.
    void EndFrame();

    // Forward SDL events to ImGui (call from input processing).
    // Returns true if ImGui wants to capture this event (mouse over UI, etc.)
    bool ProcessEvent(const SDL_Event& event);

    // Returns true if ImGui wants keyboard/mouse input (so game should ignore it)
    [[nodiscard]] bool WantsCaptureKeyboard();
    [[nodiscard]] bool WantsCaptureMouse();

} // namespace DebugUI

#else

// Stubs when ImGui is disabled
namespace DebugUI {
    inline Status Init(f32 = 16.0f) { return {}; }
    inline void Shutdown() {}
    inline void BeginFrame(i32, i32, f64) {}
    inline void EndFrame() {}
    inline bool ProcessEvent(const SDL_Event&) { return false; }
    inline bool WantsCaptureKeyboard() { return false; }
    inline bool WantsCaptureMouse() { return false; }
} // namespace DebugUI

#endif

} // namespace freak
