#pragma once

// ─────────────────────────────────────────────
// Freak Engine - Frame timer
// Provides delta time, total elapsed time, and frame counting.
// Fixed-timestep logic uses Tick(), rendering uses DeltaTime().
// ─────────────────────────────────────────────

#include <freak/core/Types.h>

namespace freak {

class FrameTimer {
public:
    // Call once at startup
    void Init();

    // Call at the start of each frame. Updates delta time and frame counter.
    void BeginFrame();

    // Seconds elapsed since last frame (variable, for rendering)
    [[nodiscard]] f64 DeltaTime() const { return m_deltaTime; }

    // Seconds elapsed since Init()
    [[nodiscard]] f64 TotalTime() const { return m_totalTime; }

    // Frames since Init()
    [[nodiscard]] u64 FrameCount() const { return m_frameCount; }

    // Instantaneous FPS (1 / deltaTime), smoothed
    [[nodiscard]] f64 FPS() const { return m_smoothFps; }

    // Fixed timestep for simulation (default 1/60)
    static constexpr f64 FixedDeltaTime = 1.0 / 60.0;

private:
    f64 m_deltaTime = 0.0;
    f64 m_totalTime = 0.0;
    f64 m_smoothFps = 0.0;
    u64 m_frameCount = 0;

    // Platform time point (using QueryPerformanceCounter on Windows via SDL)
    u64 m_lastTicks = 0;
    u64 m_frequency = 0;
};

} // namespace freak
