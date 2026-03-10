add_library(freak_framework STATIC)

target_sources(freak_framework PRIVATE
    src/Framework.cpp
)

target_include_directories(freak_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_framework
    PUBLIC
        freak_core
        freak_platform
        freak_render
        freak_scene
        freak_audio
        freak_physics
        freak_assets
)
