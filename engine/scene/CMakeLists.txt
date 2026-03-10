add_library(freak_scene STATIC)

target_sources(freak_scene PRIVATE
    src/Scene.cpp
    src/SceneLoader.cpp
)

target_include_directories(freak_scene PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_scene
    PUBLIC
        freak_core
        EnTT
        glm::glm
        nlohmann_json::nlohmann_json
)
