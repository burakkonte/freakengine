#include <freak/framework/InputActions.h>
#include <freak/platform/Input.h>
#include <freak/core/Profiler.h>

#include <SDL3/SDL_scancode.h>

namespace freak {

void UpdateInputActions(InputActions& out, const Input& raw) {
    FREAK_PROFILE_SCOPE_N("UpdateInputActions");

    // ── Movement (WASD) ──
    out.move = {0.0f, 0.0f};
    if (raw.IsKeyDown(SDL_SCANCODE_W)) out.move.y += 1.0f;
    if (raw.IsKeyDown(SDL_SCANCODE_S)) out.move.y -= 1.0f;
    if (raw.IsKeyDown(SDL_SCANCODE_D)) out.move.x += 1.0f;
    if (raw.IsKeyDown(SDL_SCANCODE_A)) out.move.x -= 1.0f;

    // ── Look (raw mouse delta) ──
    out.lookDelta = {raw.MouseDeltaX(), raw.MouseDeltaY()};

    // ── Discrete actions (pressed this frame) ──
    out.jump           = raw.IsKeyPressed(SDL_SCANCODE_SPACE);
    out.interact       = raw.IsKeyPressed(SDL_SCANCODE_E);
    out.sprint         = raw.IsKeyDown(SDL_SCANCODE_LSHIFT);
    out.crouch         = raw.IsKeyDown(SDL_SCANCODE_LCTRL);
    out.toggleDebugCam = raw.IsKeyPressed(SDL_SCANCODE_F2);
}

} // namespace freak
