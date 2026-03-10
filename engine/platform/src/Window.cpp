#include <freak/platform/Window.h>
#include <freak/core/Assert.h>
#include <freak/core/Log.h>
#include <freak/core/Profiler.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_syswm.h>

namespace freak {

Window::~Window() {
    Destroy();
}

Window::Window(Window&& other) noexcept
    : m_window(other.m_window)
    , m_width(other.m_width)
    , m_height(other.m_height) {
    other.m_window = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        Destroy();
        m_window = other.m_window;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_window = nullptr;
    }
    return *this;
}

Status Window::Create(const WindowConfig& config) {
    FREAK_PROFILE_SCOPE;

    SDL_WindowFlags flags = 0;
    if (config.fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (config.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    m_window = SDL_CreateWindow(config.title, config.width, config.height, flags);
    if (!m_window) {
        FREAK_LOG_ERROR("Platform", "Failed to create window: {}", SDL_GetError());
        return std::unexpected(Error::InitFailed);
    }

    m_width  = config.width;
    m_height = config.height;

    FREAK_LOG_INFO("Platform", "Window created: {}x{} \"{}\"", m_width, m_height, config.title);
    return {};
}

void Window::Destroy() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        FREAK_LOG_INFO("Platform", "Window destroyed");
    }
}

void Window::OnResize(i32 width, i32 height) {
    m_width  = width;
    m_height = height;
    FREAK_LOG_DEBUG("Platform", "Window resized: {}x{}", width, height);
}

void* Window::GetNativeHandle() const {
    FREAK_ASSERT(m_window != nullptr);

#ifdef FREAK_PLATFORM_WINDOWS
    SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(FREAK_PLATFORM_LINUX)
    SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
    // Try Wayland first, fall back to X11
    void* handle = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
    if (!handle) {
        // X11: the window handle is a numeric ID, not a pointer
        handle = reinterpret_cast<void*>(
            SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)
        );
    }
    return handle;
#else
    FREAK_UNREACHABLE();
    return nullptr;
#endif
}

void* Window::GetNativeDisplayType() const {
#ifdef FREAK_PLATFORM_LINUX
    FREAK_ASSERT(m_window != nullptr);
    SDL_PropertiesID props = SDL_GetWindowProperties(m_window);
    void* display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
    if (!display) {
        display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    }
    return display;
#else
    return nullptr;
#endif
}

} // namespace freak
