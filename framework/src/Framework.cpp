#include <freak/framework/Framework.h>
#include <freak/core/Log.h>

namespace freak {

Status GameplayFramework::Init() {
    FREAK_LOG_INFO("Framework", "Gameplay framework placeholder — implementation in M2");
    return {};
}

void GameplayFramework::Shutdown() {
    FREAK_LOG_INFO("Framework", "Gameplay framework shutdown");
}

} // namespace freak
