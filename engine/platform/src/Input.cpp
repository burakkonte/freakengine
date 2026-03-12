#include <freak/platform/Input.h>
#include <freak/core/Profiler.h>

#include <SDL3/SDL.h>
#include <cstring>

namespace freak {

bool Input::PollEvents() {
    FREAK_PROFILE_SCOPE;

    // Save previous frame state
    std::memcpy(m_keysPrev, m_keysDown, sizeof(m_keysDown));
    m_mouseButtonsPrev = m_mouseButtons;
    m_mouseDeltaX = 0.0f;
    m_mouseDeltaY = 0.0f;
    m_windowResized = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // TODO: Forward events to ImGui here once debug UI is integrated

        switch (event.type) {
            case SDL_EVENT_QUIT:
                return false;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < MaxKeys) {
                    m_keysDown[event.key.scancode] = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < MaxKeys) {
                    m_keysDown[event.key.scancode] = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                m_mouseX = event.motion.x;
                m_mouseY = event.motion.y;
                m_mouseDeltaX += event.motion.xrel;
                m_mouseDeltaY += event.motion.yrel;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                m_mouseButtons |= (1u << event.button.button);
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                m_mouseButtons &= ~(1u << event.button.button);
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                m_windowResized = true;
                m_windowWidth = event.window.data1;
                m_windowHeight = event.window.data2;
                break;

            default:
                break;
        }
    }

    return true;
}

bool Input::IsKeyDown(SDL_Scancode key) const {
    return (static_cast<usize>(key) < MaxKeys) && m_keysDown[key];
}

bool Input::IsKeyPressed(SDL_Scancode key) const {
    return (static_cast<usize>(key) < MaxKeys) && m_keysDown[key] && !m_keysPrev[key];
}

bool Input::IsKeyReleased(SDL_Scancode key) const {
    return (static_cast<usize>(key) < MaxKeys) && !m_keysDown[key] && m_keysPrev[key];
}

bool Input::IsMouseButtonDown(u8 button) const {
    return (m_mouseButtons & (1u << button)) != 0;
}

void Input::SetMouseCaptured(bool captured) {
    m_mouseCaptured = captured;
    SDL_Window* focused = SDL_GetKeyboardFocus();
    if (focused) {
        SDL_SetWindowRelativeMouseMode(focused, captured);
    }
}

} // namespace freak
