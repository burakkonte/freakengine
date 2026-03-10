#include <freak/assets/AssetManager.h>
#include <freak/core/Log.h>

namespace freak {

Status AssetManager::Init(StringView cookedAssetRoot) {
    m_rootPath = String(cookedAssetRoot);
    FREAK_LOG_INFO("Assets", "Asset manager placeholder — root: {}", m_rootPath);
    return {};
}

void AssetManager::Shutdown() {
    FREAK_LOG_INFO("Assets", "Asset manager shutdown");
}

} // namespace freak
