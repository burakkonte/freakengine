#pragma once

// Freak Engine - Audio System
// Placeholder for M4. Will use miniaudio for decoding/mixing.

#include <freak/core/Types.h>

namespace freak {

class AudioSystem {
public:
    Status Init();
    void Shutdown();
};

} // namespace freak
