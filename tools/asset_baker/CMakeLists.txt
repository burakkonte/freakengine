# Asset Baker - Offline tool
# Converts raw assets (glTF, textures, etc.) into cooked runtime format.
# This is a separate executable, not linked to the game runtime.
# Implementation: M1

add_executable(asset_baker)

target_sources(asset_baker PRIVATE
    src/main.cpp
)

target_link_libraries(asset_baker PRIVATE
    freak_core
    nlohmann_json::nlohmann_json
    stb
)
