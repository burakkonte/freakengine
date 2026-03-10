add_library(freak_render STATIC)

target_sources(freak_render PRIVATE
    src/RenderDevice.cpp
    src/DebugUI.cpp
    src/Camera.cpp
    src/MeshCache.cpp
    src/ForwardPass.cpp
)

target_include_directories(freak_render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_render
    PUBLIC
        freak_core
        freak_platform
        freak_scene
        bgfx
        bx
        glm::glm
)

if(FREAK_ENABLE_IMGUI)
    target_link_libraries(freak_render PUBLIC imgui)
endif()
