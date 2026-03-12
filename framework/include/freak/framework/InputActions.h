#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Input Actions
// Maps raw platform input to gameplay intentions.
// Hard-coded WASD + mouse for M2A.
// No config files, no rebinding — just a clean
// abstraction so gameplay code never touches
// SDL scancodes directly.
// ─────────────────────────────────────────────

#include <freak/core/Types.h>
#include <glm/vec2.hpp>

namespace freak {

class Input;

/// Gameplay-facing input state, updated once per frame.
struct InputActions {
    glm::vec2 move{0.0f};       // x = strafe (right+), y = forward (forward+)
    glm::vec2 lookDelta{0.0f};  // raw mouse delta in pixels
    bool jump       = false;    // pressed this frame
    bool interact   = false;    // pressed this frame
    bool sprint     = false;    // held
    bool crouch     = false;    // held
    bool toggleDebugCam = false; // pressed this frame
};

/// Read raw Input and populate gameplay actions.
/// Call once per frame before gameplay update.
void UpdateInputActions(InputActions& out, const Input& raw);

} // namespace freak
