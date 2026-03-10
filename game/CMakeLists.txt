add_executable(freakland)

target_sources(freakland PRIVATE
    src/main.cpp
)

target_include_directories(freakland PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freakland PRIVATE
    freak_core
    freak_platform
    freak_render
    freak_scene
    freak_audio
    freak_physics
    freak_assets
    freak_framework
)

# On Windows, use WinMain entry point for release builds (no console window)
if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(freakland PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Set working directory for debugging in Visual Studio / VS Code
set_target_properties(freakland PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
