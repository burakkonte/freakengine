#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Input
// Polls SDL events and tracks keyboard/mouse state.
// This is the raw input layer — gameplay uses an
// action mapping layer on top (built later in M2).
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

#include <SDL3/SDL_scancode.h>

namespace freak {

class Input {
public:
    // Process all pending SDL events. Call once per frame.
    // Returns false if the app should quit (window close, etc.)
    bool PollEvents();

    // Keyboard state
    [[nodiscard]] bool IsKeyDown(SDL_Scancode key) const;
    [[nodiscard]] bool IsKeyPressed(SDL_Scancode key) const;  // Down this frame, not last
    [[nodiscard]] bool IsKeyReleased(SDL_Scancode key) const; // Up this frame, was down

    // Mouse state
    [[nodiscard]] f32 MouseX() const { return m_mouseX; }
    [[nodiscard]] f32 MouseY() const { return m_mouseY; }
    [[nodiscard]] f32 MouseDeltaX() const { return m_mouseDeltaX; }
    [[nodiscard]] f32 MouseDeltaY() const { return m_mouseDeltaY; }
    [[nodiscard]] bool IsMouseButtonDown(u8 button) const;

    // Window state reported by events
    [[nodiscard]] bool WindowResized() const { return m_windowResized; }
    [[nodiscard]] i32 WindowWidth() const { return m_windowWidth; }
    [[nodiscard]] i32 WindowHeight() const { return m_windowHeight; }

    // Mouse capture (for first-person camera)
    void SetMouseCaptured(bool captured);
    [[nodiscard]] bool IsMouseCaptured() const { return m_mouseCaptured; }

private:
    // Current and previous frame key states
    static constexpr usize MaxKeys = 512;
    bool m_keysDown[MaxKeys] = {};
    bool m_keysPrev[MaxKeys] = {};

    // Mouse
    f32 m_mouseX = 0.0f;
    f32 m_mouseY = 0.0f;
    f32 m_mouseDeltaX = 0.0f;
    f32 m_mouseDeltaY = 0.0f;
    u32 m_mouseButtons = 0;
    u32 m_mouseButtonsPrev = 0;
    bool m_mouseCaptured = false;

    // Window
    bool m_windowResized = false;
    i32 m_windowWidth = 0;
    i32 m_windowHeight = 0;
};

} // namespace freak
