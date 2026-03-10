#pragma once

// Freak Engine - Asset Manager
// Placeholder for M1. Will handle loading cooked assets at runtime.

#include <freak/core/Types.h>

namespace freak {

class AssetManager {
public:
    Status Init(StringView cookedAssetRoot);
    void Shutdown();

private:
    String m_rootPath;
};

} // namespace freak
