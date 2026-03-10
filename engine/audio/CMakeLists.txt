add_library(freak_audio STATIC)

target_sources(freak_audio PRIVATE
    src/AudioSystem.cpp
)

target_include_directories(freak_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_audio
    PUBLIC
        freak_core
        freak_platform
)
