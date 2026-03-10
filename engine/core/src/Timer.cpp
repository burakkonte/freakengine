#include <freak/core/Timer.h>
#include <freak/core/Assert.h>

#include <chrono>

namespace freak {

using Clock = std::chrono::steady_clock;

// Convert time_point to u64 ticks for storage
static u64 NowTicks() {
    return static_cast<u64>(Clock::now().time_since_epoch().count());
}

static u64 GetFrequency() {
    // steady_clock period gives us the tick duration as a ratio
    // frequency = ticks per second
    using Period = Clock::period;
    return static_cast<u64>(Period::den) / static_cast<u64>(Period::num);
}

void FrameTimer::Init() {
    m_frequency = GetFrequency();
    FREAK_ASSERT(m_frequency > 0);
    m_lastTicks = NowTicks();
    m_deltaTime = FixedDeltaTime; // Seed first frame with a sane value
    m_totalTime = 0.0;
    m_frameCount = 0;
    m_smoothFps = 60.0;
}

void FrameTimer::BeginFrame() {
    u64 now = NowTicks();
    u64 elapsed = now - m_lastTicks;
    m_lastTicks = now;

    m_deltaTime = static_cast<f64>(elapsed) / static_cast<f64>(m_frequency);

    // Clamp to avoid spiral of death after breakpoint/hitch
    constexpr f64 maxDelta = 0.1; // 100ms max
    if (m_deltaTime > maxDelta) {
        m_deltaTime = maxDelta;
    }

    m_totalTime += m_deltaTime;
    m_frameCount++;

    // Exponential moving average for smooth FPS display
    f64 instantFps = (m_deltaTime > 0.0001) ? (1.0 / m_deltaTime) : 9999.0;
    constexpr f64 smoothing = 0.05;
    m_smoothFps = m_smoothFps + smoothing * (instantFps - m_smoothFps);
}

} // namespace freak
