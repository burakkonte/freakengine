#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Window
// Thin wrapper around SDL3 window.
// Owns the SDL_Window, provides size/state queries.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

struct SDL_Window;

namespace freak {

struct WindowConfig {
    const char* title  = "Freakland";
    i32 width          = 1280;
    i32 height         = 720;
    bool fullscreen    = false;
    bool resizable     = true;
    bool vsync         = true;
};

class Window {
public:
    Window() = default;
    ~Window();

    // Non-copyable, movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    // Create the OS window. Returns error on failure.
    [[nodiscard]] Status Create(const WindowConfig& config);

    // Destroy the window.
    void Destroy();

    // Current drawable size in pixels (accounts for DPI scaling)
    [[nodiscard]] i32 Width() const { return m_width; }
    [[nodiscard]] i32 Height() const { return m_height; }

    // Handle size changes (called from event processing)
    void OnResize(i32 width, i32 height);

    // Raw SDL handle — needed by bgfx for native window handle
    [[nodiscard]] SDL_Window* GetSDLWindow() const { return m_window; }

    // Get platform-native window handle for bgfx
    [[nodiscard]] void* GetNativeHandle() const;

    // Get native display type (X11 display, etc.) — nullptr on Windows
    [[nodiscard]] void* GetNativeDisplayType() const;

private:
    SDL_Window* m_window = nullptr;
    i32 m_width  = 0;
    i32 m_height = 0;
};

} // namespace freak
