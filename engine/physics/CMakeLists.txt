add_library(freak_physics STATIC)

target_sources(freak_physics PRIVATE
    src/PhysicsWorld.cpp
)

target_include_directories(freak_physics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freak_physics
    PUBLIC
        freak_core
)

# Jolt will be added as a dependency in M2 when collision is needed.
# For now this module compiles as an empty stub.
