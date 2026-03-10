add_library(freak_platform STATIC)

target_sources(freak_platform PRIVATE
    src/App.cpp
    src/Window.cpp
    src/Input.cpp
)

target_include_directories(freak_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_platform
    PUBLIC
        freak_core
        SDL3::SDL3
)
