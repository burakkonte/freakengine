#include <freak/core/Log.h>

#include <chrono>
#include <cstdio>
#include <mutex>

namespace freak::log {

namespace {

Level g_minLevel = Level::Trace;
std::mutex g_logMutex;

constexpr const char* LevelToString(Level level) {
    switch (level) {
        case Level::Trace: return "TRACE";
        case Level::Debug: return "DEBUG";
        case Level::Info:  return "INFO ";
        case Level::Warn:  return "WARN ";
        case Level::Error: return "ERROR";
        case Level::Fatal: return "FATAL";
    }
    return "?????";
}

// Returns elapsed seconds since Init() was called.
// We don't need wall-clock time in logs — elapsed time is more useful for debugging.
using Clock = std::chrono::steady_clock;
Clock::time_point g_startTime;

f64 ElapsedSeconds() {
    auto now = Clock::now();
    return std::chrono::duration<f64>(now - g_startTime).count();
}

} // anonymous namespace

void Init() {
    g_startTime = Clock::now();
    g_minLevel = Level::Trace;
    LogMessage(Level::Info, "Log", "Logging initialized");
}

void Shutdown() {
    LogMessage(Level::Info, "Log", "Logging shutdown");
    std::fflush(stdout);
    std::fflush(stderr);
}

void SetLevel(Level minLevel) {
    g_minLevel = minLevel;
}

void LogMessage(Level level, std::string_view category, std::string_view message) {
    if (level < g_minLevel) {
        return;
    }

    // Format: [  1.234] [INFO ] [Render] Some message here
    // Lock protects interleaved output from multiple threads.
    // This is fine for our throughput. If logging ever shows up in Tracy, revisit.
    std::lock_guard lock(g_logMutex);

    FILE* output = (level >= Level::Error) ? stderr : stdout;
    std::fprintf(output, "[%8.3f] [%s] [%-8.*s] %.*s\n",
        ElapsedSeconds(),
        LevelToString(level),
        static_cast<int>(category.size()), category.data(),
        static_cast<int>(message.size()), message.data()
    );

    if (level == Level::Fatal) {
        std::fflush(stderr);
    }
}

} // namespace freak::log
