# ─────────────────────────────────────────────
# Third-party dependencies
# All deps are git submodules in this directory.
# Each is configured here with minimal options to
# avoid pulling in unnecessary targets.
# ─────────────────────────────────────────────

# ── SDL3 ──────────────────────────────────────
# We build SDL3 as a static library to simplify distribution.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# ── bgfx (via bgfx.cmake wrapper) ────────────
# bgfx.cmake provides proper CMake targets for bgfx, bx, and bimg.
# Submodule: https://github.com/bkarber/bgfx.cmake
set(BGFX_BUILD_TOOLS    ON  CACHE BOOL "" FORCE)  # Need shaderc for shader compilation
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL        OFF CACHE BOOL "" FORCE)
set(BGFX_CUSTOM_TARGETS OFF CACHE BOOL "" FORCE)
add_subdirectory(bgfx.cmake EXCLUDE_FROM_ALL)

# bgfx/bx/bimg: strip /WX that leaks from root scope and suppress
# C4324 (struct padded due to alignment specifier) in bx headers
if(MSVC)
    foreach(_bgfx_target bx bimg bgfx)
        if(TARGET ${_bgfx_target})
            get_target_property(_opts ${_bgfx_target} COMPILE_OPTIONS)
            if(_opts)
                list(REMOVE_ITEM _opts "/WX" "/W4" "/permissive-")
                set_target_properties(${_bgfx_target} PROPERTIES COMPILE_OPTIONS "${_opts}")
            endif()
            target_compile_options(${_bgfx_target} PRIVATE /W3 /wd4324 /wd4996)
        endif()
    endforeach()
endif()

# ── EnTT ──────────────────────────────────────
# Header-only. Just needs the include path.
add_library(EnTT INTERFACE)
target_include_directories(EnTT INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/entt/single_include
)

# ── Tracy ─────────────────────────────────────
# Tracy client library. Only built when profiling is enabled.
if(FREAK_ENABLE_TRACY)
    # Tracy needs to be compiled as a static library
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)  # Don't block if profiler not connected
    add_subdirectory(tracy EXCLUDE_FROM_ALL)
else()
    # Provide a dummy target so link lines don't break
    add_library(TracyClient INTERFACE)
endif()

# ── Dear ImGui ────────────────────────────────
# ImGui has no CMakeLists.txt, so we create a target manually.
if(FREAK_ENABLE_IMGUI)
    add_library(imgui STATIC
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    )
    # ImGui needs to know about bgfx for the rendering backend
    # The actual backend files will be added in engine/render
endif()

# ── nlohmann/json ─────────────────────────────
# Header-only JSON library
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
add_subdirectory(json EXCLUDE_FROM_ALL)

# ── toml++ ────────────────────────────────────
# Header-only TOML parser
add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus/include
)

# ── GLM (math) ────────────────────────────────
# We use GLM for math rather than writing our own.
# Practical: proven, fast, well-tested, SIMD optimized.
add_subdirectory(glm EXCLUDE_FROM_ALL)

# ── stb libraries ─────────────────────────────
# stb_image for texture loading in the asset baker
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/stb
)
