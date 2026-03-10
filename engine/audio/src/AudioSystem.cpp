#include <freak/audio/AudioSystem.h>
#include <freak/core/Log.h>

namespace freak {

Status AudioSystem::Init() {
    FREAK_LOG_INFO("Audio", "Audio system placeholder — implementation in M4");
    return {};
}

void AudioSystem::Shutdown() {
    FREAK_LOG_INFO("Audio", "Audio system shutdown");
}

} // namespace freak
