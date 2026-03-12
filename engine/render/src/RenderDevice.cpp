#include <freak/render/RenderDevice.h>
#include <freak/platform/Window.h>
#include <freak/core/Assert.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>

namespace freak {

RenderDevice::~RenderDevice() {
    if (m_initialized) {
        Shutdown();
    }
}

Status RenderDevice::Init(const Window& window, const RenderConfig& config) {
    FREAK_PROFILE_SCOPE;

    FREAK_ASSERT(!m_initialized);

    m_width  = config.width;
    m_height = config.height;

    // Tell bgfx not to create its own render thread — we control the frame
    bgfx::renderFrame();

    // Platform data: connect bgfx to our SDL window
    bgfx::PlatformData pd{};
    pd.nwh = window.GetNativeHandle();
    pd.ndt = window.GetNativeDisplayType();

    bgfx::Init init;
    // rendererType == 0 means auto-detect; bgfx uses RendererType::Count for that.
    // RendererType::Noop is 0, which is NOT what we want as the default.
    init.type = (config.rendererType == 0)
        ? bgfx::RendererType::Count
        : static_cast<bgfx::RendererType::Enum>(config.rendererType);
    init.resolution.width  = static_cast<u32>(m_width);
    init.resolution.height = static_cast<u32>(m_height);
    init.platformData = pd;

    // Reset flags
    m_resetFlags = BGFX_RESET_NONE;
    if (config.vsync) {
        m_resetFlags |= BGFX_RESET_VSYNC;
    }
    init.resolution.reset = m_resetFlags;

    if (!bgfx::init(init)) {
        FREAK_LOG_ERROR("Render", "bgfx::init() failed");
        return std::unexpected(Error::InitFailed);
    }

    // Debug flags
    u32 debugFlags = BGFX_DEBUG_NONE;
    if (config.debugStats) debugFlags |= BGFX_DEBUG_STATS;
    if (config.debugText)  debugFlags |= BGFX_DEBUG_TEXT;
    bgfx::setDebug(debugFlags);

    // Set clear state for view 0 (main view)
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x1a0a2eFF, // Dark purple-blue — moody default
        1.0f,
        0
    );

    bgfx::setViewRect(0, 0, 0,
        static_cast<u16>(m_width),
        static_cast<u16>(m_height)
    );

    m_initialized = true;

    const bgfx::RendererType::Enum actualType = bgfx::getRendererType();
    FREAK_LOG_INFO("Render", "bgfx initialized: {} ({}x{})",
        bgfx::getRendererName(actualType), m_width, m_height);

    return {};
}

void RenderDevice::Shutdown() {
    if (m_initialized) {
        bgfx::shutdown();
        m_initialized = false;
        FREAK_LOG_INFO("Render", "bgfx shutdown");
    }
}

void RenderDevice::OnResize(i32 width, i32 height) {
    if (width <= 0 || height <= 0) return;

    m_width  = width;
    m_height = height;

    bgfx::reset(
        static_cast<u32>(m_width),
        static_cast<u32>(m_height),
        m_resetFlags
    );

    bgfx::setViewRect(0, 0, 0,
        static_cast<u16>(m_width),
        static_cast<u16>(m_height)
    );

    FREAK_LOG_DEBUG("Render", "Backbuffer resized: {}x{}", m_width, m_height);
}

void RenderDevice::BeginFrame() {
    FREAK_PROFILE_SCOPE_N("Render::BeginFrame");

    // Touch view 0 to ensure it renders even if nothing is submitted
    bgfx::touch(0);
}

void RenderDevice::EndFrame() {
    FREAK_PROFILE_SCOPE_N("Render::EndFrame");

    // Debug stats overlay
    bgfx::setDebug(m_showStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_NONE);

    // Submit frame to bgfx
    bgfx::frame();
}

void RenderDevice::ToggleStats() {
    m_showStats = !m_showStats;
}

} // namespace freak
