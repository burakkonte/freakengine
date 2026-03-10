#include <freak/platform/App.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <SDL3/SDL.h>

namespace freak {

Status AppInit() {
    FREAK_PROFILE_SCOPE;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        FREAK_LOG_FATAL("Platform", "SDL_Init failed: {}", SDL_GetError());
        return std::unexpected(Error::InitFailed);
    }

    FREAK_LOG_INFO("Platform", "SDL initialized (video, audio, gamepad)");
    return {};
}

void AppShutdown() {
    SDL_Quit();
    FREAK_LOG_INFO("Platform", "SDL shutdown");
}

} // namespace freak
