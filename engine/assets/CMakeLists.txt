add_library(freak_assets STATIC)

target_sources(freak_assets PRIVATE
    src/AssetManager.cpp
)

target_include_directories(freak_assets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_assets
    PUBLIC
        freak_core
        nlohmann_json::nlohmann_json
)
