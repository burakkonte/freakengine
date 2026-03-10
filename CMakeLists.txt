cmake_minimum_required(VERSION 3.28)

project(Freakland
    VERSION 0.1.0
    LANGUAGES CXX C
    DESCRIPTION "Freak Engine - Custom runtime for Freakland"
)

# ─────────────────────────────────────────────
# Global C++ standard
# ─────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ─────────────────────────────────────────────
# Build configuration options
# ─────────────────────────────────────────────
option(FREAK_ENABLE_TRACY    "Enable Tracy profiler integration" ON)
option(FREAK_ENABLE_ASSERTS  "Enable runtime assertions"        ON)
option(FREAK_ENABLE_IMGUI    "Enable ImGui debug overlay"       ON)

# ─────────────────────────────────────────────
# Output directories
# ─────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ─────────────────────────────────────────────
# Compiler warnings — strict from day one
# ─────────────────────────────────────────────
if(MSVC)
    add_compile_options(
        /W4         # High warning level
        /WX         # Warnings as errors
        /permissive- # Standards conformance
        /Zc:__cplusplus # Report correct __cplusplus value
        /utf-8      # Source and execution charset
        /MP         # Multi-process compilation
    )
    # Suppress overly noisy MSVC warnings we've reviewed and accepted
    add_compile_options(
        /wd4201     # nameless struct/union (used in math types)
    )
else()
    add_compile_options(
        -Wall -Wextra -Wpedantic -Werror
        -Wno-unused-parameter   # Too noisy during early development
    )
endif()

# ─────────────────────────────────────────────
# Global defines
# ─────────────────────────────────────────────
if(FREAK_ENABLE_TRACY)
    add_compile_definitions(TRACY_ENABLE)
endif()

if(FREAK_ENABLE_ASSERTS)
    add_compile_definitions(FREAK_ASSERTS_ENABLED)
endif()

if(FREAK_ENABLE_IMGUI)
    add_compile_definitions(FREAK_IMGUI_ENABLED)
endif()

# Platform detection
if(WIN32)
    add_compile_definitions(FREAK_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(FREAK_PLATFORM_LINUX)
endif()

# ─────────────────────────────────────────────
# Third-party dependencies
# ─────────────────────────────────────────────
add_subdirectory(third_party)

# ─────────────────────────────────────────────
# Engine modules (dependency order)
# ─────────────────────────────────────────────
add_subdirectory(engine/core)
add_subdirectory(engine/platform)
add_subdirectory(engine/assets)
add_subdirectory(engine/scene)
add_subdirectory(engine/render)
add_subdirectory(engine/audio)
add_subdirectory(engine/physics)

# ─────────────────────────────────────────────
# Gameplay framework
# ─────────────────────────────────────────────
add_subdirectory(framework)

# ─────────────────────────────────────────────
# Game executable
# ─────────────────────────────────────────────
add_subdirectory(game)

# ─────────────────────────────────────────────
# Tools (offline, not linked to game)
# ─────────────────────────────────────────────
add_subdirectory(tools/asset_baker)
