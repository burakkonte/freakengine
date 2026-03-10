#pragma once

// Freak Engine - Physics World
// Placeholder for M2. Will wrap Jolt for collision queries.

#include <freak/core/Types.h>

namespace freak {

class PhysicsWorld {
public:
    Status Init();
    void Shutdown();
};

} // namespace freak
